#include "Bazooka.hpp"

#include "Projectile.hpp"

namespace dungeon {

namespace {

constexpr int kMagazineSize = 1;
constexpr float kCooldownSeconds = 0.45f;

}

WeaponType Bazooka::type() const { return WeaponType::Bazooka; }

std::string_view Bazooka::displayName() const { return "Bazooka"; }

void Bazooka::update(float deltaSeconds)
{
    if (cooldownRemaining_ > 0.0f) {
        cooldownRemaining_ -= deltaSeconds;
    }
}

void Bazooka::attack(sf::Vector2f origin, sf::Vector2f direction)
{
    (void)tryShoot(origin, direction);
}

std::optional<Projectile> Bazooka::tryShoot(sf::Vector2f origin, sf::Vector2f direction)
{
    if (cooldownRemaining_ > 0.0f || ammo_ <= 0) {
        return std::nullopt;
    }

    --ammo_;
    cooldownRemaining_ = kCooldownSeconds;
    return makeBazookaProjectile(origin, direction);
}

int Bazooka::ammo() const { return ammo_; }

int Bazooka::magazineSize() const { return kMagazineSize; }

void Bazooka::reload()
{
    ammo_ = kMagazineSize;
}

}
