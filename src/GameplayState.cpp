#include "GameplayState.hpp"

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
    projectileShape_.setRadius(5.0f);
    projectileShape_.setOrigin({5.0f, 5.0f});
    projectileShape_.setFillColor(sf::Color(255, 230, 90));
    setupTargetEntities();
}

void GameplayState::onEnter() {}
void GameplayState::onExit() {}

void GameplayState::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
        player_.dodgeRoll(readMovementInput());
    } else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        const sf::Vector2i mousePixel{event.mouseButton.x, event.mouseButton.y};
        const sf::Vector2f mouseWorld = context_.window->mapPixelToCoords(mousePixel);
        fireProjectile(mouseWorld);
    }
}

void GameplayState::update(float deltaSeconds)
{
    player_.update(deltaSeconds);
    updatePlayer(deltaSeconds);
    updateProjectiles(deltaSeconds);
}

void GameplayState::render(sf::RenderTarget& target)
{
    map_.render(target);
    for (const auto& entity : targetEntities_) {
        target.draw(entity);
    }
    for (const auto& projectile : projectiles_) {
        projectileShape_.setRadius(projectile.radius);
        projectileShape_.setOrigin({projectile.radius, projectile.radius});
        projectileShape_.setPosition(projectile.position);
        target.draw(projectileShape_);
    }
    player_.render(target);
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

            return projectile.lifetimeSeconds <= 0.0f
                || map_.collides(bounds)
                || projectileHitsEntity(bounds);
        }),
        projectiles_.end());
}

void GameplayState::fireProjectile(sf::Vector2f target)
{
    const sf::Vector2f direction = normalized(target - player_.position());
    if (direction.x == 0.0f && direction.y == 0.0f) {
        return;
    }

    Projectile projectile;
    projectile.position = player_.position();
    projectile.velocity = direction * 440.0f;
    projectile.damage = {10.0f, DamageType::Bullet, false};
    projectile.lifetimeSeconds = 2.0f;
    projectile.radius = 5.0f;
    projectiles_.push_back(projectile);
}

void GameplayState::setupTargetEntities()
{
    sf::RectangleShape target({28.0f, 28.0f});
    target.setOrigin(target.getSize() / 2.0f);
    target.setPosition({map_.tileSize() * 18.0f, map_.tileSize() * 9.0f});
    target.setFillColor(sf::Color(210, 85, 85));
    targetEntities_.push_back(target);
}

sf::Vector2f GameplayState::readMovementInput() const
{
    sf::Vector2f direction{};

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        direction.y -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        direction.y += 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        direction.x -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        direction.x += 1.0f;
    }

    return normalized(direction);
}

bool GameplayState::projectileHitsEntity(const sf::FloatRect& bounds) const
{
    return std::any_of(targetEntities_.begin(), targetEntities_.end(), [&bounds](const sf::RectangleShape& entity) {
        return entity.getGlobalBounds().intersects(bounds);
    });
}

}
