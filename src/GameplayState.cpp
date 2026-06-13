#include "GameplayState.hpp"
#include "Bazooka.hpp"
#include "Laser.hpp"
#include "PauseState.hpp"
#include "PostGameStatsState.hpp"
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
    if (endScreenShown_) {
        return;
    }

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
    if (endScreenShown_) {
        return;
    }

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

    if (!player_.isAlive()) {
        requestEndGame(PostGameResult::Lose);
    }

    showPendingEndGame();
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
    if (finalBoss_) {
        drawFinalBossLaser(target);
        target.draw(finalBoss_->body);
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
    renderBossHealthBar(target);
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
            finalBoss_.reset();
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

    if (isFinalRoom(roomIndex)) {
        spawnFinalBoss(roomIndex);
        return;
    }

    const int roomBonus = static_cast<int>(roomIndex);
    const int triangleCount = 2 + roomBonus;
    const int squareCount = 2 + roomBonus;
    const int pentagonCount = roomIndex > 4 ? 2 : 1;
    const int circleCount = 1;

    if (currentWave_ == 1) {
        addEnemies(EnemyType::Shooter, triangleCount, roomIndex);
        addEnemies(EnemyType::Chaser, squareCount, roomIndex);
        return;
    }

    addEnemies(EnemyType::Shooter, triangleCount, roomIndex);
    addEnemies(EnemyType::Chaser, squareCount, roomIndex);
    addEnemies(EnemyType::Laser, pentagonCount, roomIndex);
    addEnemies(EnemyType::HeavyRanged, circleCount, roomIndex);
}

void GameplayState::addEnemies(EnemyType type, int count, std::size_t roomIndex)
{
    for (int i = 0; i < count; ++i) {
        const sf::Vector2f spawn = randomSpawnPosition(roomIndex);

        if (type == EnemyType::Shooter) {
            enemies_.push_back(Enemy::createRanged(spawn));
        } else if (type == EnemyType::Chaser) {
            enemies_.push_back(Enemy::createMelee(spawn));
        } else if (type == EnemyType::HeavyRanged) {
            enemies_.push_back(Enemy::createHeavyRanged(spawn));
        } else if (type == EnemyType::Laser) {
            enemies_.push_back(Enemy::createLaser(spawn));
        }
    }
}

sf::Vector2f GameplayState::randomSpawnPosition(std::size_t roomIndex)
{
    const sf::FloatRect room = map_.roomBounds(roomIndex);
    const sf::Vector2f fallback = map_.roomCenter(roomIndex);

    if (room.width <= 96.0f || room.height <= 96.0f) {
        return fallback;
    }

    for (int attempt = 0; attempt < 200; ++attempt) {
        const int randomX = std::rand() % static_cast<int>(room.width - 96.0f);
        const int randomY = std::rand() % static_cast<int>(room.height - 96.0f);

        sf::Vector2f position{
            room.left + 48.0f + static_cast<float>(randomX),
            room.top + 48.0f + static_cast<float>(randomY)
        };

        sf::FloatRect enemyBounds{position.x - 22.0f, position.y - 22.0f, 44.0f, 44.0f};
        if (map_.collides(enemyBounds)) {
            continue;
        }

        if (distance(position, player_.position()) < 160.0f) {
            continue;
        }

        return position;
    }

    return fallback;
}

void GameplayState::spawnFinalBoss(std::size_t roomIndex)
{
    FinalBoss boss;
    boss.body.setRadius(56.0f);
    boss.body.setPointCount(8);
    boss.body.setOrigin({56.0f, 56.0f});
    boss.body.setPosition(map_.roomCenter(roomIndex));
    boss.body.setFillColor(sf::Color(190, 25, 25));
    boss.body.setOutlineColor(sf::Color::White);
    boss.body.setOutlineThickness(3.0f);
    boss.state = BossAttackState::Cooldown;
    boss.stateTimer = 1.0f;
    finalBoss_ = boss;
}

void GameplayState::updateFinalBoss(float deltaSeconds)
{
    if (!finalBoss_) {
        return;
    }

    FinalBoss& boss = *finalBoss_;
    boss.stateTimer -= deltaSeconds;

    switch (boss.state) {
    case BossAttackState::Cooldown:
        if (boss.stateTimer <= 0.0f) {
            chooseFinalBossAttack();
        }
        break;

    case BossAttackState::RangedBurst:
        boss.shootTimer -= deltaSeconds;
        if (boss.shootTimer <= 0.0f) {
            const sf::Vector2f direction = normalized(player_.position() - boss.body.getPosition());
            if (direction.x != 0.0f || direction.y != 0.0f) {
                projectiles_.push_back(makeEnemyProjectile(boss.body.getPosition(), direction));
            }
            boss.shootTimer = 0.16f;
        }
        if (boss.stateTimer <= 0.0f) {
            boss.state = BossAttackState::Cooldown;
            boss.stateTimer = 0.75f;
        }
        break;

    case BossAttackState::Dash: {
        const sf::Vector2f movement = boss.dashDirection * 520.0f * deltaSeconds;
        const sf::Vector2f oldPosition = boss.body.getPosition();
        boss.body.move(movement);

        if (map_.collides(boss.body.getGlobalBounds()) ||
            distance(boss.body.getPosition(), boss.dashTarget) < 12.0f ||
            boss.stateTimer <= 0.0f) {
            if (map_.collides(boss.body.getGlobalBounds())) {
                boss.body.setPosition(oldPosition);
            }
            boss.state = BossAttackState::Cooldown;
            boss.stateTimer = 0.85f;
        }

        if (!boss.dashHitPlayer && boss.body.getGlobalBounds().intersects(player_.bounds())) {
            player_.applyDamage({20.0f, DamageType::Melee, false});
            boss.dashHitPlayer = true;
        }
        break;
    }

    case BossAttackState::LaserWarning:
        if (boss.stateTimer <= 0.0f) {
            boss.state = BossAttackState::LaserBeam;
            boss.stateTimer = 1.0f;
            boss.laserHitPlayer = false;
        }
        break;

    case BossAttackState::LaserBeam:
        if (!boss.laserHitPlayer &&
            laserHitsRectangle(boss.body.getPosition(), boss.laserDirection, 1400.0f, player_.bounds())) {
            player_.applyDamage({20.0f, DamageType::Laser, false});
            boss.laserHitPlayer = true;
        }
        if (boss.stateTimer <= 0.0f) {
            boss.state = BossAttackState::Cooldown;
            boss.stateTimer = 0.9f;
        }
        break;
    }
}

void GameplayState::chooseFinalBossAttack()
{
    if (!finalBoss_) {
        return;
    }

    FinalBoss& boss = *finalBoss_;
    const int randomAttack = std::rand() % 3;

    switch (randomAttack) {
    case 0:
        boss.state = BossAttackState::RangedBurst;
        boss.stateTimer = 2.4f;
        boss.shootTimer = 0.0f;
        break;

    case 1:
        boss.state = BossAttackState::Dash;
        boss.stateTimer = 1.2f;
        boss.dashHitPlayer = false;
        // The dash target is a coordinate snapshot: the boss reads the player's
        // exact position once here, then keeps dashing toward that saved point.
        boss.dashTarget = player_.position();
        boss.dashDirection = normalized(boss.dashTarget - boss.body.getPosition());
        break;

    default:
        boss.state = BossAttackState::LaserWarning;
        boss.stateTimer = 1.5f;
        boss.laserHitPlayer = false;
        boss.laserDirection = normalized(player_.position() - boss.body.getPosition());
        if (boss.laserDirection.x == 0.0f && boss.laserDirection.y == 0.0f) {
            boss.laserDirection = {1.0f, 0.0f};
        }
        break;
    }
}

void GameplayState::drawFinalBossLaser(sf::RenderTarget& target)
{
    if (!finalBoss_) {
        return;
    }

    const FinalBoss& boss = *finalBoss_;
    float thickness = 0.0f;
    sf::Color color = sf::Color::Transparent;

    if (boss.state == BossAttackState::LaserWarning) {
        thickness = 4.0f;
        color = sf::Color(230, 20, 20, 170);
    } else if (boss.state == BossAttackState::LaserBeam) {
        thickness = 40.0f;
        color = sf::Color(255, 230, 20, 220);
    } else {
        return;
    }

    sf::RectangleShape laser({1400.0f, thickness});
    laser.setOrigin({0.0f, thickness * 0.5f});
    laser.setPosition(boss.body.getPosition());
    laser.setRotation(std::atan2(boss.laserDirection.y, boss.laserDirection.x) * 180.0f / kPi);
    laser.setFillColor(color);
    target.draw(laser);
}

bool GameplayState::isFinalRoom(std::size_t roomIndex) const
{
    return map_.roomCount() > 0 && roomIndex == map_.roomCount() - 1;
}

sf::FloatRect GameplayState::bossBounds() const
{
    if (!finalBoss_) {
        return {};
    }

    return finalBoss_->body.getGlobalBounds();
}

void GameplayState::damageBoss(const Damage& damage)
{
    if (!finalBoss_) {
        return;
    }

    finalBoss_->health -= damage.amount;
    if (finalBoss_->health > 0.0f) {
        return;
    }

    finalBoss_.reset();
    map_.unlockDoors();
    currentWave_ = 3;
    waitingForWeaponPickup_ = false;
    requestEndGame(PostGameResult::Win);

    if (currentRoomIndex_ && *currentRoomIndex_ < roomEncounters_.size()) {
        roomEncounters_[*currentRoomIndex_].cleared = true;
    }
}

void GameplayState::requestEndGame(PostGameResult result)
{
    if (!pendingEndGame_) {
        pendingEndGame_ = result;
    }
}

void GameplayState::showPendingEndGame()
{
    if (endScreenShown_ || !pendingEndGame_) {
        return;
    }

    endScreenShown_ = true;
    if (context_.showEndGame) {
        context_.showEndGame(*pendingEndGame_ == PostGameResult::Win);
    }
}

void GameplayState::updateEnemies(float deltaSeconds)
{
    updateFinalBoss(deltaSeconds);

    for (auto& enemy : enemies_) {
        enemy.updateAgainstPlayer(player_.position(), deltaSeconds, projectiles_);

        if (enemy.enemyType() == EnemyType::Chaser && enemy.bounds().intersects(player_.bounds())) {
            player_.applyDamage({5.0f, DamageType::Melee, false});
        }

        if (enemy.laserCanDamagePlayer(player_.bounds())) {
            player_.applyDamage({15.0f, DamageType::Laser, false});
            enemy.markLaserHitPlayer();
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
                if (finalBoss_ && bounds.intersects(bossBounds())) {
                    if (projectile.kind == ProjectileKind::Bazooka) {
                        createExplosion(projectile.position);
                    } else {
                        damageBoss(projectile.damage);
                    }
                    return true;
                }

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

    if (finalBoss_) {
        return;
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

    if (isFinalRoom(roomIndex)) {
        waveText_.setString("Final Boss");
    } else {
        waveText_.setString("Wave " + std::to_string(waveNumber));
    }
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
    if (finalBoss_ && distance(position, rectangleCenter(bossBounds())) <= 120.0f) {
        damageBoss({30.0f, DamageType::Explosion, false});
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
    if (finalBoss_ && laserHitsRectangle(laserShot.position, direction, length, bossBounds())) {
        damageBoss(laserShot.damage);
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

void GameplayState::renderBossHealthBar(sf::RenderTarget& target)
{
    if (!finalBoss_) {
        return;
    }

    const sf::View previousView = target.getView();
    target.setView(target.getDefaultView());

    const float barWidth = 360.0f;
    const float barHeight = 18.0f;
    const sf::Vector2u windowSize = context_.window->getSize();
    const sf::Vector2f position{
        static_cast<float>(windowSize.x) * 0.5f - barWidth * 0.5f,
        48.0f
    };

    sf::RectangleShape outline({barWidth, barHeight});
    outline.setPosition(position);
    outline.setFillColor(sf::Color::Transparent);
    outline.setOutlineColor(sf::Color::White);
    outline.setOutlineThickness(2.0f);

    // Health bar formula:
    // current health divided by maximum health gives a value from 0 to 1.
    // Multiplying by the full bar width converts that percentage into pixels.
    const float healthPercent = std::max(0.0f, finalBoss_->health / finalBoss_->maxHealth);
    sf::RectangleShape inner({barWidth * healthPercent, barHeight});
    inner.setPosition(position);
    inner.setFillColor(sf::Color(220, 35, 35));

    target.draw(inner);
    target.draw(outline);
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
