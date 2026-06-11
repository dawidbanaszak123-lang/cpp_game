#include "Projectile.hpp"

namespace dungeon {

Projectile makePlayerProjectile(sf::Vector2f origin, sf::Vector2f direction, float damageAmount)
{
    Projectile projectile;
    projectile.position = origin;
    projectile.velocity = direction * 520.0f;
    projectile.damage = {damageAmount, DamageType::Bullet, false};
    projectile.lifetimeSeconds = 2.0f;
    projectile.radius = 5.0f;
    projectile.owner = ProjectileOwner::Player;
    return projectile;
}

Projectile makeEnemyProjectile(sf::Vector2f origin, sf::Vector2f direction)
{
    Projectile projectile;
    projectile.position = origin;
    projectile.velocity = direction * 260.0f;
    projectile.damage = {8.0f, DamageType::Bullet, false};
    projectile.lifetimeSeconds = 3.0f;
    projectile.radius = 5.0f;
    projectile.owner = ProjectileOwner::Enemy;
    return projectile;
}

}
