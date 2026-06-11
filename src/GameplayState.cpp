#include "GameplayState.hpp"

#include "Revolver.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <algorithm>
#include <cmath>

namespace dungeon {

namespace {

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

    const sf::Vector2f spawn = map_.spawnPosition();
    enemies_.push_back(Enemy::createRanged(spawn + sf::Vector2f{240.0f, 80.0f}));
    enemies_.push_back(Enemy::createMelee(spawn + sf::Vector2f{120.0f, -110.0f}));

    projectileShape_.setRadius(5.0f);
    projectileShape_.setOrigin({5.0f, 5.0f});

    crosshairHorizontal_.setSize({24.0f, 2.0f});
    crosshairHorizontal_.setOrigin({12.0f, 1.0f});
    crosshairHorizontal_.setFillColor(sf::Color::White);
    crosshairVertical_.setSize({2.0f, 24.0f});
    crosshairVertical_.setOrigin({1.0f, 12.0f});
    crosshairVertical_.setFillColor(sf::Color::White);
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
    }
}

void GameplayState::update(float deltaSeconds)
{
    player_.update(deltaSeconds);
    updatePlayer(deltaSeconds);
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        const sf::Vector2i mousePixel = sf::Mouse::getPosition(*context_.window);
        const sf::Vector2f mouseWorld = context_.window->mapPixelToCoords(mousePixel, cameraView());
        fireProjectile(mouseWorld);
    }
    updateEnemies(deltaSeconds);
    updateProjectiles(deltaSeconds);
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

void GameplayState::updateEnemies(float deltaSeconds)
{
    for (auto& enemy : enemies_) {
        enemy.updateAgainstPlayer(player_.position(), deltaSeconds, projectiles_);

        if (enemy.enemyType() == EnemyType::Chaser && enemy.bounds().intersects(player_.bounds())) {
            player_.applyDamage({12.0f * deltaSeconds, DamageType::Melee, false});
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
