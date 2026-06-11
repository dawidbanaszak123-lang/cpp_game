#include "Rifle.hpp"

#include "Projectile.hpp"

namespace dungeon {

namespace {

constexpr int kMagazineSize = 30;
constexpr float kCooldownSeconds = 0.12f;
constexpr float kDamage = 3.0f;

}

WeaponType Rifle::type() const { return WeaponType::Rifle; }

std::string_view Rifle::displayName() const { return "Rifle"; }

void Rifle::update(float deltaSeconds)
{
    if (cooldownRemaining_ > 0.0f) {
        cooldownRemaining_ -= deltaSeconds;
    }
}

void Rifle::attack(sf::Vector2f origin, sf::Vector2f direction)
{
    (void)tryShoot(origin, direction);
}

std::optional<Projectile> Rifle::tryShoot(sf::Vector2f origin, sf::Vector2f direction)
{
    if (cooldownRemaining_ > 0.0f || ammo_ <= 0) {
        return std::nullopt;
    }

    --ammo_;
    cooldownRemaining_ = kCooldownSeconds;
    return makePlayerProjectile(origin, direction, kDamage);
}

int Rifle::ammo() const { return ammo_; }

int Rifle::magazineSize() const { return kMagazineSize; }

void Rifle::reload()
{
    ammo_ = kMagazineSize;
}

}
