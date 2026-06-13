#include "Laser.hpp"
#include "Projectile.hpp"

namespace dungeon {

namespace {

constexpr float kCooldownSeconds = 1.5f;

}

WeaponType Laser::type() const { return WeaponType::Laser; }

std::string_view Laser::displayName() const { return "Laser"; }

void Laser::update(float deltaSeconds)
{
    if (cooldownRemaining_ > 0.0f) {
        cooldownRemaining_ -= deltaSeconds;
    }
}

void Laser::attack(sf::Vector2f origin, sf::Vector2f direction)
{
    (void)tryShoot(origin, direction);
}

std::optional<Projectile> Laser::tryShoot(sf::Vector2f origin, sf::Vector2f direction)
{
    if (cooldownRemaining_ > 0.0f) {
        return std::nullopt;
    }

    cooldownRemaining_ = kCooldownSeconds;
    return makeLaserShot(origin, direction);
}

int Laser::ammo() const { return 1; }

int Laser::magazineSize() const { return 1; }

void Laser::reload() {}

}
