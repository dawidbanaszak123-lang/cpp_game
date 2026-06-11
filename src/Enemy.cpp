#include "Enemy.hpp"

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

[[nodiscard]] std::unique_ptr<sf::Shape> makeShooterShape()
{
    auto shape = std::make_unique<sf::CircleShape>(18.0f, 3);
    shape->setOrigin({18.0f, 18.0f});
    shape->setFillColor(sf::Color(40, 210, 90));
    return shape;
}

[[nodiscard]] std::unique_ptr<sf::Shape> makeChaserShape()
{
    auto shape = std::make_unique<sf::RectangleShape>(sf::Vector2f{30.0f, 30.0f});
    shape->setOrigin({15.0f, 15.0f});
    shape->setFillColor(sf::Color(255, 105, 190));
    return shape;
}

}

Enemy::Enemy(EnemyType type)
    : type_(type)
{
    if (type_ == EnemyType::Shooter) {
        body_ = makeShooterShape();
        health_ = 30.0f;
        maxHealth_ = 30.0f;
    } else {
        body_ = makeChaserShape();
        health_ = 20.0f;
        maxHealth_ = 20.0f;
    }
}

Enemy Enemy::createRanged(sf::Vector2f position)
{
    Enemy enemy(EnemyType::Shooter);
    enemy.setPosition(position);
    return enemy;
}

Enemy Enemy::createMelee(sf::Vector2f position)
{
    Enemy enemy(EnemyType::Chaser);
    enemy.setPosition(position);
    return enemy;
}

void Enemy::update(float deltaSeconds)
{
    if (shootCooldownRemaining_ > 0.0f) {
        shootCooldownRemaining_ -= deltaSeconds;
    }
}

void Enemy::render(sf::RenderTarget& target)
{
    target.draw(*body_);
}

sf::Vector2f Enemy::position() const
{
    return body_->getPosition();
}

void Enemy::setPosition(sf::Vector2f position)
{
    body_->setPosition(position);
}

void Enemy::applyDamage(const Damage& damage)
{
    health_ -= damage.amount;
    if (health_ < 0.0f) {
        health_ = 0.0f;
    }
}

bool Enemy::isAlive() const
{
    return health_ > 0.0f;
}

float Enemy::health() const
{
    return health_;
}

float Enemy::maxHealth() const
{
    return maxHealth_;
}

EnemyType Enemy::enemyType() const
{
    return type_;
}

void Enemy::chooseBehavior(float deltaSeconds)
{
    update(deltaSeconds);
}

void Enemy::updateAgainstPlayer(sf::Vector2f playerPosition, float deltaSeconds, std::vector<Projectile>& projectiles)
{
    update(deltaSeconds);

    if (type_ == EnemyType::Shooter) {
        updateShooter(playerPosition, deltaSeconds, projectiles);
    } else if (type_ == EnemyType::Chaser) {
        updateChaser(playerPosition, deltaSeconds);
    }
}

sf::FloatRect Enemy::bounds() const
{
    return body_->getGlobalBounds();
}

sf::Vector2f Enemy::directionTo(sf::Vector2f target) const
{
    // Direction is target minus current position. Normalizing keeps only the
    // direction, so speed and damage are independent of distance to the player.
    return normalized(target - position());
}

void Enemy::updateShooter(sf::Vector2f playerPosition, float, std::vector<Projectile>& projectiles)
{
    if (shootCooldownRemaining_ > 0.0f) {
        return;
    }

    const sf::Vector2f direction = directionTo(playerPosition);
    if (direction.x == 0.0f && direction.y == 0.0f) {
        return;
    }

    projectiles.push_back(makeEnemyProjectile(position(), direction));
    shootCooldownRemaining_ = shootCooldownSeconds_;
}

void Enemy::updateChaser(sf::Vector2f playerPosition, float deltaSeconds)
{
    const sf::Vector2f direction = directionTo(playerPosition);
    // The melee enemy tracks the player by moving a small step along the
    // normalized direction every frame.
    body_->move(direction * moveSpeed_ * deltaSeconds);
}

}
