#include "Bazooka.hpp"
#include "Projectile.hpp"

namespace dungeon {

namespace {

constexpr int kMagazineSize = 1;
constexpr float kCooldownSeconds = 0.45f;

}//widoczne tylko tu stałe

WeaponType Bazooka::type() const { return WeaponType::Bazooka; }

std::string_view Bazooka::displayName() const { return "Bazooka"; }

void Bazooka::update(float deltaSeconds)
{
    if (cooldownRemaining_ > 0.0f) {
        cooldownRemaining_ -= deltaSeconds;
    }// cooldown między strzałami o czas klatki
}

void Bazooka::attack(sf::Vector2f origin, sf::Vector2f direction)
{
    (void)tryShoot(origin, direction);
}

std::optional<Projectile> Bazooka::tryShoot(sf::Vector2f origin, sf::Vector2f direction)
{
    if (cooldownRemaining_ > 0.0f || ammo_ <= 0) {
        return std::nullopt;
    }// Blokada strzału gdy cooldown lub nie ma amunicji

    --ammo_;
    cooldownRemaining_ = kCooldownSeconds;
    return makeBazookaProjectile(origin, direction);// Pobiera amunicje, reset timera odnowienia i generowanie pocisku
}

int Bazooka::ammo() const { return ammo_; }//ile naboji zostało

int Bazooka::magazineSize() const { return kMagazineSize; }//ile maksymalnie naboji sie miesci

void Bazooka::reload()
{
    ammo_ = kMagazineSize;//znowu maks amunicji
}

}
