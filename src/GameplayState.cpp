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
}

void GameplayState::onEnter() {}
void GameplayState::onExit() {}

void GameplayState::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
        player_.dodgeRoll(readMovementInput());
    } else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        const sf::Vector2i mousePixel{event.mouseButton.x, event.mouseButton.y};
        const sf::Vector2f mouseWorld = context_.window->mapPixelToCoords(mousePixel, cameraView());
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
    const sf::View previousView = target.getView();
    target.setView(cameraView());

    map_.render(target);
    for (const auto& projectile : projectiles_) {
        projectileShape_.setRadius(projectile.radius);
        projectileShape_.setOrigin({projectile.radius, projectile.radius});
        projectileShape_.setPosition(projectile.position);
        target.draw(projectileShape_);
    }
    player_.render(target);

    target.setView(previousView);
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
                || map_.collides(bounds);
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
