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
    projectile.kind = ProjectileKind::Normal;
    return projectile;
}

Projectile makeEnemyProjectile(sf::Vector2f origin, sf::Vector2f direction)
{
    Projectile projectile;
    projectile.position = origin;
    projectile.velocity = direction * 260.0f;
    projectile.damage = {5.0f, DamageType::Bullet, false};
    projectile.lifetimeSeconds = 3.0f;
    projectile.radius = 5.0f;
    projectile.owner = ProjectileOwner::Enemy;
    projectile.kind = ProjectileKind::Normal;
    return projectile;
}

Projectile makeHeavyEnemyProjectile(sf::Vector2f origin, sf::Vector2f direction)
{
    Projectile projectile;
    projectile.position = origin;
    projectile.velocity = direction * 150.0f;
    projectile.damage = {20.0f, DamageType::Bullet, false};
    projectile.lifetimeSeconds = 4.0f;
    projectile.radius = 11.0f;
    projectile.owner = ProjectileOwner::Enemy;
    projectile.kind = ProjectileKind::Normal;
    return projectile;
}

Projectile makeBazookaProjectile(sf::Vector2f origin, sf::Vector2f direction)
{
    Projectile projectile;
    projectile.position = origin;
    projectile.velocity = direction * 360.0f;
    projectile.damage = {0.0f, DamageType::Explosion, false};
    projectile.lifetimeSeconds = 3.0f;
    projectile.radius = 8.0f;
    projectile.owner = ProjectileOwner::Player;
    projectile.kind = ProjectileKind::Bazooka;
    return projectile;
}

Projectile makeLaserShot(sf::Vector2f origin, sf::Vector2f direction)
{
    Projectile projectile;
    projectile.position = origin;
    projectile.velocity = direction;
    projectile.damage = {15.0f, DamageType::Laser, false};
    projectile.lifetimeSeconds = 0.08f;
    projectile.radius = 0.0f;
    projectile.owner = ProjectileOwner::Player;
    projectile.kind = ProjectileKind::Laser;
    return projectile;
}

}
