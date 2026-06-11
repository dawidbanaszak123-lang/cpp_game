#include "Player.hpp"

namespace dungeon {

Player::Player()
{
    body_.setSize({28.0f, 28.0f});
    body_.setOrigin(body_.getSize() / 2.0f);
    body_.setFillColor(sf::Color(60, 160, 255));
}

void Player::update(float deltaSeconds)
{
    if (dodgeTimer_ > 0.0f) {
        dodgeTimer_ -= deltaSeconds;
    }
    if (dodgeCooldownTimer_ > 0.0f) {
        dodgeCooldownTimer_ -= deltaSeconds;
    }
    if (invincibilityTimer_ > 0.0f) {
        invincibilityTimer_ -= deltaSeconds;
    }

    body_.setFillColor(sf::Color(60, 160, 255));
}

void Player::render(sf::RenderTarget& target)
{
    target.draw(body_);
}

sf::Vector2f Player::position() const
{
    return body_.getPosition();
}

void Player::setPosition(sf::Vector2f position)
{
    body_.setPosition(position);
}

void Player::applyDamage(const Damage& damage)
{
    if (hasInvincibilityFrames()) {
        return;
    }

    health_ -= damage.amount;
    if (health_ < 0.0f) {
        health_ = 0.0f;
    }
}

bool Player::isAlive() const
{
    return health_ > 0.0f;
}

float Player::health() const
{
    return health_;
}

float Player::maxHealth() const
{
    return maxHealth_;
}

void Player::move(sf::Vector2f displacement)
{
    if (displacement.x != 0.0f || displacement.y != 0.0f) {
        lastMoveDirection_ = displacement;
    }
    body_.move(displacement);
}

void Player::dodgeRoll(sf::Vector2f direction)
{
    if (!canDodge()) {
        return;
    }

    if (direction.x == 0.0f && direction.y == 0.0f) {
        direction = lastMoveDirection_;
    }

    lastMoveDirection_ = direction;
    dodgeTimer_ = dodgeDurationSeconds_;
    dodgeCooldownTimer_ = dodgeCooldownSeconds_;
    invincibilityTimer_ = invincibilityDurationSeconds_;
}

bool Player::hasInvincibilityFrames() const
{
    return invincibilityTimer_ > 0.0f;
}

void Player::meleeAttack(sf::Vector2f) {}

void Player::fireRangedWeapon(sf::Vector2f) {}

void Player::equipMeleeWeapon(std::unique_ptr<IMeleeWeapon> weapon)
{
    meleeWeapon_ = std::move(weapon);
}

void Player::addRangedWeapon(std::unique_ptr<IRangedWeapon> weapon)
{
    rangedWeapons_.push_back(std::move(weapon));
}

sf::FloatRect Player::bounds() const
{
    return body_.getGlobalBounds();
}

float Player::movementSpeed() const
{
    return isDodging() ? baseSpeed_ * dodgeSpeedMultiplier_ : baseSpeed_;
}

bool Player::isDodging() const
{
    return dodgeTimer_ > 0.0f;
}

bool Player::canDodge() const
{
    return dodgeCooldownTimer_ <= 0.0f;
}

}
