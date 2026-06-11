#include "GameplayState.hpp"

#include "Rifle.hpp"
#include "Revolver.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <algorithm>
#include <cmath>
#include <string>

namespace dungeon {

namespace {

constexpr const char* kSystemFontPath = "C:/Windows/Fonts/arial.ttf";

[[nodiscard]] sf::Vector2f normalized(sf::Vector2f vector)
{
    const float length = std::sqrt(vector.x * vector.x + vector.y * vector.y);
    if (length <= 0.0f) {
        return {};
    }

    return {vector.x / length, vector.y / length};
}

}

GameplayState::GameplayState(GameContext& context)
    : context_(context)
{
    map_.generateRoom(25, 18);
    player_.setPosition(map_.spawnPosition());
    player_.addRangedWeapon(std::make_unique<Revolver>());
    player_.addRangedWeapon(std::make_unique<Rifle>());

    roomEncounters_.resize(map_.roomCount());
    currentRoomIndex_ = map_.roomContaining(player_.bounds());
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
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
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
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        const sf::Vector2i mousePixel = sf::Mouse::getPosition(*context_.window);
        const sf::Vector2f mouseWorld = context_.window->mapPixelToCoords(mousePixel, cameraView());
        fireProjectile(mouseWorld);
    }
    updateEnemies(deltaSeconds);
    updateProjectiles(deltaSeconds);

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
        if (*roomIndex < roomEncounters_.size() && !roomEncounters_[*roomIndex].visited) {
            roomEncounters_[*roomIndex].visited = true;
            map_.lockRoomDoors(*roomIndex);
            spawnRoomEnemies(*roomIndex);
        }
    }

    if (*roomIndex < roomEncounters_.size() && roomEncounters_[*roomIndex].visited && !roomEncounters_[*roomIndex].cleared && enemies_.empty()) {
        roomEncounters_[*roomIndex].cleared = true;
        map_.unlockDoors();
        projectiles_.clear();
    }
}

void GameplayState::spawnRoomEnemies(std::size_t roomIndex)
{
    enemies_.clear();
    projectiles_.clear();

    const sf::Vector2f center = map_.roomCenter(roomIndex);
    enemies_.push_back(Enemy::createMelee(center + sf::Vector2f{-120.0f, -80.0f}));
    enemies_.push_back(Enemy::createRanged(center + sf::Vector2f{120.0f, 80.0f}));

    if (roomIndex % 2 == 0) {
        enemies_.push_back(Enemy::createMelee(center + sf::Vector2f{80.0f, -130.0f}));
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
                return true;
            }

            if (projectile.owner == ProjectileOwner::Enemy && bounds.intersects(player_.bounds())) {
                player_.applyDamage(projectile.damage);
                return true;
            }

            if (projectile.owner == ProjectileOwner::Player) {
                for (auto& enemy : enemies_) {
                    if (bounds.intersects(enemy.bounds())) {
                        enemy.applyDamage(projectile.damage);
                        return true;
                    }
                }
            }

            return false;
        }),
        projectiles_.end());
}

void GameplayState::fireProjectile(sf::Vector2f target)
{
    if (auto projectile = player_.tryFireRangedWeapon(target)) {
        projectiles_.push_back(*projectile);
    }
}

void GameplayState::renderHud(sf::RenderTarget& target)
{
    const sf::View previousView = target.getView();
    target.setView(target.getDefaultView());

    hpText_.setString(
        "HP: " + std::to_string(static_cast<int>(player_.health())) +
        " / " + std::to_string(static_cast<int>(player_.maxHealth())) +
        " | Ammo: " + std::to_string(player_.activeAmmo()) +
        " / " + std::to_string(player_.activeMagazineSize()) +
        (player_.isReloading() ? " | RELOADING..." : ""));
    target.draw(hpText_);

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
