#include "Enemy.hpp"
#include <algorithm>
#include <cmath>

namespace dungeon {

namespace {

[[nodiscard]] sf::Vector2f normalized(sf::Vector2f vector)
{
    const float length = std::sqrt(vector.x * vector.x + vector.y * vector.y);
    if (length <= 0.0f) {
        return {};
    }// Normalizacja wektora do długości jednostkowej i blokada przed dzieleniem przez zero

    return {vector.x / length, vector.y / length};
}


[[nodiscard]] float distance(sf::Vector2f a, sf::Vector2f b)
{
    const sf::Vector2f difference = a - b;
    return std::sqrt(difference.x * difference.x + difference.y * difference.y);
} // odległośc między dwoma punktami na oknie

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

[[nodiscard]] std::unique_ptr<sf::Shape> makeHeavyRangedShape()
{
    auto shape = std::make_unique<sf::CircleShape>(20.0f);
    shape->setOrigin({20.0f, 20.0f});
    shape->setFillColor(sf::Color(245, 145, 35));
    return shape;
}

[[nodiscard]] std::unique_ptr<sf::Shape> makeLaserShape()
{
    auto shape = std::make_unique<sf::CircleShape>(20.0f, 5);
    shape->setOrigin({20.0f, 20.0f});
    shape->setFillColor(sf::Color(170, 80, 255));
    return shape;
}

}
//statystyki
Enemy::Enemy(EnemyType type)
    : type_(type)
{
    if (type_ == EnemyType::Shooter) {
        body_ = makeShooterShape();
        health_ = 30.0f;
        maxHealth_ = 30.0f;
    } else if (type_ == EnemyType::Chaser) {
        body_ = makeChaserShape();
        health_ = 20.0f;
        maxHealth_ = 20.0f;
    } else if (type_ == EnemyType::HeavyRanged) {
        body_ = makeHeavyRangedShape();
        health_ = 55.0f;
        maxHealth_ = 55.0f;
        shootCooldownSeconds_ = 2.0f;
    } else {
        body_ = makeLaserShape();
        health_ = 60.0f;
        maxHealth_ = 60.0f;
        laserCooldownRemaining_ = 1.0f;
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

Enemy Enemy::createHeavyRanged(sf::Vector2f position)
{
    Enemy enemy(EnemyType::HeavyRanged);
    enemy.setPosition(position);
    return enemy;
}

Enemy Enemy::createLaser(sf::Vector2f position)
{
    Enemy enemy(EnemyType::Laser);
    enemy.setPosition(position);
    return enemy;
}

void Enemy::update(float deltaSeconds)//cooldown ale dla enemy
{
    if (shootCooldownRemaining_ > 0.0f) {
        shootCooldownRemaining_ -= deltaSeconds;
    }
    if (laserCooldownRemaining_ > 0.0f) {
        laserCooldownRemaining_ -= deltaSeconds;
    }
}

void Enemy::render(sf::RenderTarget& target)//render dla lasera
{
    if (type_ == EnemyType::Laser && laserState_ == 1) {
        drawLaser(target, sf::Color(220, 30, 30, 160), 6.0f);
    } else if (type_ == EnemyType::Laser && laserState_ == 2) {
        drawLaser(target, sf::Color(255, 230, 20, 210), 24.0f);
    }

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

void Enemy::applyDamage(const Damage& damage)// Odejmowanie punktów życia o wartość obrażeń z blokadą do zera
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
    } else if (type_ == EnemyType::HeavyRanged) {
        updateHeavyRanged(playerPosition, deltaSeconds, projectiles);
    } else if (type_ == EnemyType::Laser) {
        updateLaser(playerPosition, deltaSeconds);
    } else if (type_ == EnemyType::Chaser) {
        updateChaser(playerPosition, deltaSeconds);
    }
}// Wywołanie aktualizacji timerów i rozpoczecia akcji przypisanej

sf::FloatRect Enemy::bounds() const
{
    return body_->getGlobalBounds();
}

bool Enemy::laserCanDamagePlayer(const sf::FloatRect& playerBounds) const
{
    if (type_ != EnemyType::Laser || laserState_ != 2 || laserHitPlayer_) {
        return false;
    }

    const sf::Vector2f center = rectangleCenter(playerBounds);
    const float laserLength = 700.0f;
    float projection = dot(center - position(), laserDirection_);

    if (projection < 0.0f || projection > laserLength) {
        return false;
    }

    const sf::Vector2f closestPoint = position() + laserDirection_ * projection;
    const float playerRadius = std::max(playerBounds.width, playerBounds.height) * 0.5f;
    return distance(center, closestPoint) <= playerRadius + 12.0f;
}//sprawdza czy laser trafil i zdejmuje hp

void Enemy::markLaserHitPlayer()
{
    laserHitPlayer_ = true;
}

sf::Vector2f Enemy::directionTo(sf::Vector2f target) const
{
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

void Enemy::updateHeavyRanged(sf::Vector2f playerPosition, float, std::vector<Projectile>& projectiles)
{
    if (shootCooldownRemaining_ > 0.0f) {
        return;
    }

    const sf::Vector2f direction = directionTo(playerPosition);
    if (direction.x == 0.0f && direction.y == 0.0f) {
        return;
    }

    projectiles.push_back(makeHeavyEnemyProjectile(position(), direction));
    shootCooldownRemaining_ = shootCooldownSeconds_;
}

void Enemy::updateLaser(sf::Vector2f playerPosition, float deltaSeconds)
{
    if (laserState_ == 0) {
        if (laserCooldownRemaining_ <= 0.0f) {
            startLaserCharge(playerPosition);
        }
        return;
    }

    laserStateTimer_ -= deltaSeconds;

    if (laserState_ == 1 && laserStateTimer_ <= 0.0f) {
        laserState_ = 2;
        laserStateTimer_ = 1.0f;
        laserHitPlayer_ = false;
    } else if (laserState_ == 2 && laserStateTimer_ <= 0.0f) {
        laserState_ = 0;
        laserCooldownRemaining_ = 2.0f;
    }
}

void Enemy::updateChaser(sf::Vector2f playerPosition, float deltaSeconds)
{
    const sf::Vector2f direction = directionTo(playerPosition);
    body_->move(direction * moveSpeed_ * deltaSeconds);
}

void Enemy::startLaserCharge(sf::Vector2f playerPosition)
{
    const sf::Vector2f direction = directionTo(playerPosition);
    if (direction.x != 0.0f || direction.y != 0.0f) {
        laserDirection_ = direction;
    }

    laserState_ = 1;
    laserStateTimer_ = 1.5f;
    laserHitPlayer_ = false;
}

void Enemy::drawLaser(sf::RenderTarget& target, sf::Color color, float thickness)
{
    sf::RectangleShape laser({700.0f, thickness});
    laser.setOrigin({0.0f, thickness / 2.0f});
    laser.setPosition(position());
    laser.setRotation(std::atan2(laserDirection_.y, laserDirection_.x) * 180.0f / 3.14159265f);
    laser.setFillColor(color);
    target.draw(laser);
}

}
