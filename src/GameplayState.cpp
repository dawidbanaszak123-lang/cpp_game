#include "GameplayState.hpp"

#include "Bazooka.hpp"
#include "Laser.hpp"
#include "PauseState.hpp"
#include "Rifle.hpp"
#include "Revolver.hpp"
#include "StateStack.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <string>

namespace dungeon {

namespace {

constexpr const char* kSystemFontPath = "C:/Windows/Fonts/arial.ttf";
constexpr float kPi = 3.14159265f;

[[nodiscard]] sf::Vector2f normalized(sf::Vector2f vector)
{
    const float length = std::sqrt(vector.x * vector.x + vector.y * vector.y);
    if (length <= 0.0f) {
        return {};
    }

    return {vector.x / length, vector.y / length};
}

[[nodiscard]] float distance(sf::Vector2f a, sf::Vector2f b)
{
    const sf::Vector2f difference = a - b;
    return std::sqrt(difference.x * difference.x + difference.y * difference.y);
}

[[nodiscard]] float dot(sf::Vector2f a, sf::Vector2f b)
{
    return a.x * b.x + a.y * b.y;
}

[[nodiscard]] sf::Vector2f rectangleCenter(const sf::FloatRect& rectangle)
{
    return {
        rectangle.left + rectangle.width * 0.5f,
        rectangle.top + rectangle.height * 0.5f
    };
}

[[nodiscard]] bool laserHitsRectangle(sf::Vector2f start, sf::Vector2f direction, float length, const sf::FloatRect& rectangle)
{
    const sf::Vector2f center = rectangleCenter(rectangle);
    float projection = dot(center - start, direction);
    if (projection < 0.0f || projection > length) {
        return false;
    }

    const sf::Vector2f closestPoint = start + direction * projection;
    const float rectangleRadius = std::max(rectangle.width, rectangle.height) * 0.5f + 4.0f;
    return distance(center, closestPoint) <= rectangleRadius;
}

}

GameplayState::GameplayState(GameContext& context)
    : context_(context)
{
    map_.generateRoom(25, 18);
    player_.setPosition(map_.spawnPosition());
    player_.addRangedWeapon(std::make_unique<Revolver>());
    player_.addRangedWeapon(std::make_unique<Rifle>());
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    roomEncounters_.resize(map_.roomCount());
    currentRoomIndex_ = map_.roomContaining(player_.bounds());
    startingRoomIndex_ = currentRoomIndex_;
    if (currentRoomIndex_) {
        roomEncounters_[*currentRoomIndex_].visited = true;
        roomEncounters_[*currentRoomIndex_].cleared = true;
    }

    projectileShape_.setRadius(5.0f);
    projectileShape_.setOrigin({5.0f, 5.0f});

    crosshairHorizontal_.setSize({24.0f, 2.0f});
    crosshairHorizontal_.setOrigin({12.0f, 1.0f});
    crosshairHorizontal_.setFillColor(sf::Color::White);
    crosshairVertical_.setSize({2.0f, 24.0f});
    crosshairVertical_.setOrigin({1.0f, 12.0f});
    crosshairVertical_.setFillColor(sf::Color::White);

    hudFont_.loadFromFile(kSystemFontPath);
    hpText_.setFont(hudFont_);
    hpText_.setCharacterSize(22);
    hpText_.setFillColor(sf::Color::White);
    hpText_.setPosition({16.0f, 12.0f});

    waveText_.setFont(hudFont_);
    waveText_.setCharacterSize(48);
    waveText_.setFillColor(sf::Color::White);

    weaponSpawnText_.setFont(hudFont_);
    weaponSpawnText_.setCharacterSize(22);
    weaponSpawnText_.setFillColor(sf::Color::Yellow);
    weaponSpawnText_.setPosition({460.0f, 12.0f});
}

void GameplayState::onEnter()
{
    context_.window->setMouseCursorVisible(false);
}

void GameplayState::onExit()
{
    context_.window->setMouseCursorVisible(true);
}

void GameplayState::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
        context_.states->push(std::make_unique<PauseState>(context_));
    } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
        player_.dodgeRoll(readMovementInput());
    } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Num1) {
        player_.selectRangedWeapon(0);
    } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Num2) {
        player_.selectRangedWeapon(1);
    } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R) {
        player_.startReload();
    }
}

void GameplayState::update(float deltaSeconds)
{
    player_.update(deltaSeconds);
    updatePlayer(deltaSeconds);
    updateRoomEncounters();
    updateWeaponPickup();
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        const sf::Vector2i mousePixel = sf::Mouse::getPosition(*context_.window);
        const sf::Vector2f mouseWorld = context_.window->mapPixelToCoords(mousePixel, cameraView());
        fireProjectile(mouseWorld);
    }
    updateEnemies(deltaSeconds);
    updateProjectiles(deltaSeconds);
    updateEffects(deltaSeconds);
    updateWaves(deltaSeconds);

    if (!player_.isAlive() && context_.returnToMainMenu) {
        context_.returnToMainMenu();
    }
}

void GameplayState::render(sf::RenderTarget& target)
{
    const sf::View previousView = target.getView();
    target.setView(cameraView());

    map_.render(target);
    for (const auto& projectile : projectiles_) {
        projectileShape_.setRadius(projectile.radius);
        projectileShape_.setOrigin({projectile.radius, projectile.radius});
        projectileShape_.setPosition(projectile.position);
        projectileShape_.setPointCount(projectile.owner == ProjectileOwner::Player ? 8 : 30);
        projectileShape_.setFillColor(projectile.owner == ProjectileOwner::Player ? sf::Color::White : sf::Color::Yellow);
        target.draw(projectileShape_);
    }
    for (const auto& laserEffect : laserEffects_) {
        target.draw(laserEffect.line);
    }
    for (const auto& explosion : explosions_) {
        target.draw(explosion.circle);
        for (const auto& line : explosion.lines) {
            target.draw(line.shape);
        }
    }
    if (weaponPickup_) {
        target.draw(weaponPickup_->shape);
    }
    for (auto& enemy : enemies_) {
        enemy.render(target);
    }
    player_.render(target);

    target.setView(previousView);
    renderHud(target);
    renderCrosshair(target);
}

bool GameplayState::allowsUnderlyingUpdate() const { return false; }
bool GameplayState::allowsUnderlyingRender() const { return false; }

void GameplayState::updatePlayer(float deltaSeconds)
{
    const sf::Vector2f direction = readMovementInput();
    if (direction.x == 0.0f && direction.y == 0.0f) {
        return;
    }

    const sf::Vector2f displacement = direction * player_.movementSpeed() * deltaSeconds;
    const sf::Vector2f originalPosition = player_.position();

    player_.move({displacement.x, 0.0f});
    if (map_.collides(player_.bounds())) {
        player_.setPosition(originalPosition);
    }

    const sf::Vector2f afterX = player_.position();
    player_.move({0.0f, displacement.y});
    if (map_.collides(player_.bounds())) {
        player_.setPosition(afterX);
    }
}

void GameplayState::updateRoomEncounters()
{
    const std::optional<std::size_t> roomIndex = map_.roomContaining(player_.bounds());
    if (!roomIndex) {
        return;
    }

    if (currentRoomIndex_ != roomIndex) {
        currentRoomIndex_ = roomIndex;

        if (*roomIndex >= roomEncounters_.size()) {
            return;
        }

        if (startingRoomIndex_ && *roomIndex == *startingRoomIndex_) {
            enemies_.clear();
            projectiles_.clear();
            currentWave_ = 0;
            waitingForWeaponPickup_ = false;
            weaponPickup_.reset();
            roomEncounters_[*roomIndex].visited = true;
            roomEncounters_[*roomIndex].cleared = true;
            map_.unlockDoors();
            return;
        }

        if (!roomEncounters_[*roomIndex].visited) {
            roomEncounters_[*roomIndex].visited = true;
            roomEncounters_[*roomIndex].cleared = false;
            startWave(1);
        }
    }
}

void GameplayState::spawnRoomEnemies(std::size_t roomIndex)
{
    enemies_.clear();
    projectiles_.clear();

    const sf::Vector2f center = map_.roomCenter(roomIndex);

    if (currentWave_ == 1) {
        enemies_.push_back(Enemy::createMelee(center + sf::Vector2f{-120.0f, -80.0f}));
        enemies_.push_back(Enemy::createRanged(center + sf::Vector2f{120.0f, 80.0f}));
    } else if (currentWave_ == 2) {
        enemies_.push_back(Enemy::createMelee(center + sf::Vector2f{-150.0f, -90.0f}));
        enemies_.push_back(Enemy::createMelee(center + sf::Vector2f{120.0f, -120.0f}));
        enemies_.push_back(Enemy::createRanged(center + sf::Vector2f{150.0f, 90.0f}));
        enemies_.push_back(Enemy::createRanged(center + sf::Vector2f{-100.0f, 110.0f}));
    }
}

void GameplayState::updateEnemies(float deltaSeconds)
{
    for (auto& enemy : enemies_) {
        enemy.updateAgainstPlayer(player_.position(), deltaSeconds, projectiles_);

        if (enemy.bounds().intersects(player_.bounds())) {
            player_.applyDamage({5.0f, DamageType::Melee, false});
        }
    }

    enemies_.erase(
        std::remove_if(enemies_.begin(), enemies_.end(), [](const Enemy& enemy) {
            return !enemy.isAlive();
        }),
        enemies_.end());
}

void GameplayState::updateProjectiles(float deltaSeconds)
{
    for (auto& projectile : projectiles_) {
        projectile.position += projectile.velocity * deltaSeconds;
        projectile.lifetimeSeconds -= deltaSeconds;
    }

    projectiles_.erase(
        std::remove_if(projectiles_.begin(), projectiles_.end(), [this](const Projectile& projectile) {
            sf::FloatRect bounds{
                projectile.position.x - projectile.radius,
                projectile.position.y - projectile.radius,
                projectile.radius * 2.0f,
                projectile.radius * 2.0f
            };

            if (projectile.lifetimeSeconds <= 0.0f || map_.collides(bounds)) {
                if (projectile.kind == ProjectileKind::Bazooka) {
                    createExplosion(projectile.position);
                }
                return true;
            }

            if (projectile.owner == ProjectileOwner::Enemy && bounds.intersects(player_.bounds())) {
                player_.applyDamage(projectile.damage);
                return true;
            }

            if (projectile.owner == ProjectileOwner::Player) {
                for (auto& enemy : enemies_) {
                    if (bounds.intersects(enemy.bounds())) {
                        if (projectile.kind == ProjectileKind::Bazooka) {
                            createExplosion(projectile.position);
                        } else {
                            enemy.applyDamage(projectile.damage);
                        }
                        return true;
                    }
                }
            }

            return false;
        }),
        projectiles_.end());
}

void GameplayState::updateEffects(float deltaSeconds)
{
    for (auto& laserEffect : laserEffects_) {
        laserEffect.lifetimeSeconds -= deltaSeconds;
    }

    laserEffects_.erase(
        std::remove_if(laserEffects_.begin(), laserEffects_.end(), [](const LaserEffect& laserEffect) {
            return laserEffect.lifetimeSeconds <= 0.0f;
        }),
        laserEffects_.end());

    for (auto& explosion : explosions_) {
        explosion.lifetimeSeconds -= deltaSeconds;
        const float progress = 1.0f - explosion.lifetimeSeconds / explosion.maxLifetimeSeconds;
        const float radius = 15.0f + progress * 75.0f;
        explosion.circle.setRadius(radius);
        explosion.circle.setOrigin({radius, radius});

        sf::Color color = sf::Color(255, 160, 30, 150);
        float alpha = 150.0f * (1.0f - progress);
        if (alpha < 0.0f) {
            alpha = 0.0f;
        }
        color.a = static_cast<sf::Uint8>(alpha);
        explosion.circle.setFillColor(sf::Color::Transparent);
        explosion.circle.setOutlineColor(color);
        explosion.circle.setOutlineThickness(3.0f);

        for (auto& line : explosion.lines) {
            line.lifetimeSeconds -= deltaSeconds;
            line.shape.move(line.velocity * deltaSeconds);

            if (!line.dealtDamage) {
                for (auto& enemy : enemies_) {
                    if (line.shape.getGlobalBounds().intersects(enemy.bounds())) {
                        enemy.applyDamage({5.0f, DamageType::Explosion, false});
                        line.dealtDamage = true;
                        break;
                    }
                }
            }
        }

        explosion.lines.erase(
            std::remove_if(explosion.lines.begin(), explosion.lines.end(), [](const ExplosionLine& line) {
                return line.lifetimeSeconds <= 0.0f;
            }),
            explosion.lines.end());
    }

    explosions_.erase(
        std::remove_if(explosions_.begin(), explosions_.end(), [](const ExplosionEffect& explosion) {
            return explosion.lifetimeSeconds <= 0.0f;
        }),
        explosions_.end());
}

void GameplayState::updateWaves(float deltaSeconds)
{
    if (waveTextTimer_ > 0.0f) {
        waveTextTimer_ -= deltaSeconds;
    }
    if (weaponSpawnTextTimer_ > 0.0f) {
        weaponSpawnTextTimer_ -= deltaSeconds;
    }

    if (currentWave_ == 1 && enemies_.empty() && !waitingForWeaponPickup_ && !weaponPickup_) {
        startWave(2);
        return;
    }

    if (currentWave_ == 2 && enemies_.empty() && !waitingForWeaponPickup_ && !weaponPickup_) {
        waitingForWeaponPickup_ = true;
        map_.unlockDoors();
        projectiles_.clear();

        if (currentRoomIndex_ && *currentRoomIndex_ < roomEncounters_.size()) {
            roomEncounters_[*currentRoomIndex_].cleared = true;
        }

        currentWave_ = 3;
        spawnRandomWeapon();
    }
}

void GameplayState::updateWeaponPickup()
{
    if (!weaponPickup_) {
        return;
    }

    if (!weaponPickup_->shape.getGlobalBounds().intersects(player_.bounds())) {
        return;
    }

    const WeaponType pickedType = weaponPickup_->type;
    player_.replaceActiveRangedWeapon(makeWeapon(pickedType));
    weaponPickup_.reset();
    waitingForWeaponPickup_ = false;
}

void GameplayState::fireProjectile(sf::Vector2f target)
{
    if (auto projectile = player_.tryFireRangedWeapon(target)) {
        if (projectile->kind == ProjectileKind::Laser) {
            fireLaser(*projectile);
        } else {
            projectiles_.push_back(*projectile);
        }
    }
}

void GameplayState::startWave(int waveNumber)
{
    currentWave_ = waveNumber;
    waitingForWeaponPickup_ = false;
    weaponPickup_.reset();
    enemies_.clear();
    projectiles_.clear();

    currentRoomIndex_ = map_.roomContaining(player_.bounds());
    const std::size_t roomIndex = currentRoomIndex_.value_or(0);
    map_.lockRoomDoors(roomIndex);
    spawnRoomEnemies(roomIndex);

    waveText_.setString("Wave " + std::to_string(waveNumber));
    const sf::FloatRect textBounds = waveText_.getLocalBounds();
    const sf::Vector2u windowSize = context_.window->getSize();
    waveText_.setOrigin({textBounds.left + textBounds.width * 0.5f, textBounds.top + textBounds.height * 0.5f});
    waveText_.setPosition({static_cast<float>(windowSize.x) * 0.5f, static_cast<float>(windowSize.y) * 0.5f});
    waveTextTimer_ = 2.0f;
}

void GameplayState::spawnRandomWeapon()
{
    int randomNumber = std::rand() % 3;
    WeaponType type = WeaponType::Bazooka;
    std::string name = "Bazooka";

    if (randomNumber == 1) {
        type = WeaponType::Rifle;
        name = "Rifle";
    } else if (randomNumber == 2) {
        type = WeaponType::Laser;
        name = "Laser";
    }

    const std::size_t roomIndex = currentRoomIndex_.value_or(0);
    WeaponPickup pickup;
    pickup.type = type;
    pickup.name = name;
    pickup.shape.setSize({34.0f, 22.0f});
    pickup.shape.setOrigin({17.0f, 11.0f});
    pickup.shape.setPosition(map_.roomCenter(roomIndex));
    pickup.shape.setFillColor(sf::Color(255, 210, 70));
    pickup.shape.setOutlineColor(sf::Color::White);
    pickup.shape.setOutlineThickness(2.0f);
    weaponPickup_ = pickup;

    weaponSpawnText_.setString(name + " Spawned!");
    weaponSpawnTextTimer_ = 3.0f;
}

void GameplayState::createExplosion(sf::Vector2f position)
{
    ExplosionEffect explosion;
    explosion.circle.setPosition(position);
    explosion.circle.setRadius(15.0f);
    explosion.circle.setOrigin({15.0f, 15.0f});
    explosion.circle.setFillColor(sf::Color::Transparent);
    explosion.circle.setOutlineColor(sf::Color(255, 160, 30, 150));
    explosion.circle.setOutlineThickness(3.0f);
    explosion.lifetimeSeconds = 0.45f;
    explosion.maxLifetimeSeconds = 0.45f;

    for (auto& enemy : enemies_) {
        if (distance(position, rectangleCenter(enemy.bounds())) <= 90.0f) {
            enemy.applyDamage({30.0f, DamageType::Explosion, false});
        }
    }

    for (int i = 0; i < 16; ++i) {
        const float angle = static_cast<float>(i) * 2.0f * kPi / 16.0f;
        const sf::Vector2f direction{std::cos(angle), std::sin(angle)};

        ExplosionLine line;
        line.shape.setSize({22.0f, 4.0f});
        line.shape.setOrigin({11.0f, 2.0f});
        line.shape.setPosition(position);
        line.shape.setRotation(angle * 180.0f / kPi);
        line.shape.setFillColor(sf::Color(255, 230, 90));
        line.velocity = direction * 240.0f;
        line.lifetimeSeconds = 0.35f;
        explosion.lines.push_back(line);
    }

    explosions_.push_back(explosion);
}

void GameplayState::fireLaser(const Projectile& laserShot)
{
    const sf::Vector2f direction = normalized(laserShot.velocity);
    const float length = 1400.0f;

    for (auto& enemy : enemies_) {
        if (laserHitsRectangle(laserShot.position, direction, length, enemy.bounds())) {
            enemy.applyDamage(laserShot.damage);
        }
    }

    LaserEffect laserEffect;
    laserEffect.line.setSize({length, 4.0f});
    laserEffect.line.setOrigin({0.0f, 2.0f});
    laserEffect.line.setPosition(laserShot.position);
    laserEffect.line.setRotation(std::atan2(direction.y, direction.x) * 180.0f / kPi);
    laserEffect.line.setFillColor(sf::Color(80, 220, 255, 190));
    laserEffect.lifetimeSeconds = 0.08f;
    laserEffects_.push_back(laserEffect);
}

std::unique_ptr<IRangedWeapon> GameplayState::makeWeapon(WeaponType type) const
{
    if (type == WeaponType::Bazooka) {
        return std::make_unique<Bazooka>();
    }
    if (type == WeaponType::Laser) {
        return std::make_unique<Laser>();
    }

    return std::make_unique<Rifle>();
}

void GameplayState::renderHud(sf::RenderTarget& target)
{
    const sf::View previousView = target.getView();
    target.setView(target.getDefaultView());

    hpText_.setString(
        "HP: " + std::to_string(static_cast<int>(player_.health())) +
        " / " + std::to_string(static_cast<int>(player_.maxHealth())) +
        " | Weapon: " + std::string(player_.activeWeaponName()) +
        " | Ammo: " + std::to_string(player_.activeAmmo()) +
        " / " + std::to_string(player_.activeMagazineSize()) +
        (player_.isReloading() ? " | RELOADING..." : ""));
    target.draw(hpText_);
    if (waveTextTimer_ > 0.0f) {
        target.draw(waveText_);
    }
    if (weaponSpawnTextTimer_ > 0.0f) {
        target.draw(weaponSpawnText_);
    }

    target.setView(previousView);
}

void GameplayState::renderCrosshair(sf::RenderTarget& target)
{
    const sf::View previousView = target.getView();
    target.setView(target.getDefaultView());

    const sf::Vector2i mousePixel = sf::Mouse::getPosition(*context_.window);
    const sf::Vector2f mousePosition{static_cast<float>(mousePixel.x), static_cast<float>(mousePixel.y)};
    crosshairHorizontal_.setPosition(mousePosition);
    crosshairVertical_.setPosition(mousePosition);
    target.draw(crosshairHorizontal_);
    target.draw(crosshairVertical_);

    target.setView(previousView);
}

sf::View GameplayState::cameraView() const
{
    const sf::Vector2u windowSize = context_.window->getSize();
    sf::View view(sf::FloatRect(0.0f, 0.0f, static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)));
    view.setCenter(player_.position());
    return view;
}

sf::Vector2f GameplayState::readMovementInput() const
{
    sf::Vector2f direction{};

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        direction.y -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        direction.y += 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        direction.x -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        direction.x += 1.0f;
    }

    return normalized(direction);
}
}
