#include "Revolver.hpp"

#include "Projectile.hpp"

namespace dungeon {

namespace {

constexpr int kMagazineSize = 6;
constexpr float kCooldownSeconds = 0.55f;
constexpr float kDamage = 32.0f;

}

WeaponType Revolver::type() const { return WeaponType::Revolver; }

std::string_view Revolver::displayName() const { return "Revolver"; }

void Revolver::update(float deltaSeconds)
{
    if (cooldownRemaining_ > 0.0f) {
        cooldownRemaining_ -= deltaSeconds;
    }
}

void Revolver::attack(sf::Vector2f origin, sf::Vector2f direction)
{
    (void)tryShoot(origin, direction);
}

std::optional<Projectile> Revolver::tryShoot(sf::Vector2f origin, sf::Vector2f direction)
{
    if (cooldownRemaining_ > 0.0f || ammo_ <= 0) {
        return std::nullopt;
    }

    --ammo_;
    cooldownRemaining_ = kCooldownSeconds;
    return makePlayerProjectile(origin, direction, kDamage);
}

int Revolver::ammo() const { return ammo_; }

int Revolver::magazineSize() const { return kMagazineSize; }

void Revolver::reload()
{
    ammo_ = kMagazineSize;
}

}
