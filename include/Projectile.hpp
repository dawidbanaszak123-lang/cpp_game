#pragma once

#include "Damage.hpp"

#include <SFML/System/Vector2.hpp>

namespace dungeon {

enum class ProjectileOwner {
    Player,
    Enemy
};

enum class ProjectileKind {
    Normal,
    Bazooka,
    Laser
};

struct Projectile {
    sf::Vector2f position{};
    sf::Vector2f velocity{};
    Damage damage{};
    float lifetimeSeconds{};
    float radius{5.0f};
    ProjectileOwner owner{ProjectileOwner::Player};
    ProjectileKind kind{ProjectileKind::Normal};
};

[[nodiscard]] Projectile makePlayerProjectile(sf::Vector2f origin, sf::Vector2f direction, float damageAmount);
[[nodiscard]] Projectile makeEnemyProjectile(sf::Vector2f origin, sf::Vector2f direction);
[[nodiscard]] Projectile makeBazookaProjectile(sf::Vector2f origin, sf::Vector2f direction);
[[nodiscard]] Projectile makeLaserShot(sf::Vector2f origin, sf::Vector2f direction);

}
