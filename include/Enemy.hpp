#pragma once

#include "Character.hpp"
#include "Projectile.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <memory>
#include <vector>

namespace dungeon {

enum class EnemyType {
    Chaser,
    Shooter,
    HeavyRanged,
    Laser
};

class Enemy : public Character {
public:
    explicit Enemy(EnemyType type);

    [[nodiscard]] static Enemy createRanged(sf::Vector2f position);
    [[nodiscard]] static Enemy createMelee(sf::Vector2f position);
    [[nodiscard]] static Enemy createHeavyRanged(sf::Vector2f position);
    [[nodiscard]] static Enemy createLaser(sf::Vector2f position);

    void update(float deltaSeconds) override;
    void render(sf::RenderTarget& target) override;

    [[nodiscard]] sf::Vector2f position() const override;
    void setPosition(sf::Vector2f position) override;
    void applyDamage(const Damage& damage) override;
    [[nodiscard]] bool isAlive() const override;
    [[nodiscard]] float health() const override;
    [[nodiscard]] float maxHealth() const override;

    [[nodiscard]] EnemyType enemyType() const;
    void chooseBehavior(float deltaSeconds);
    void updateAgainstPlayer(sf::Vector2f playerPosition, float deltaSeconds, std::vector<Projectile>& projectiles);
    [[nodiscard]] sf::FloatRect bounds() const;
    [[nodiscard]] bool laserCanDamagePlayer(const sf::FloatRect& playerBounds) const;
    void markLaserHitPlayer();

private:
    [[nodiscard]] sf::Vector2f directionTo(sf::Vector2f target) const;
    void updateShooter(sf::Vector2f playerPosition, float deltaSeconds, std::vector<Projectile>& projectiles);
    void updateHeavyRanged(sf::Vector2f playerPosition, float deltaSeconds, std::vector<Projectile>& projectiles);
    void updateLaser(sf::Vector2f playerPosition, float deltaSeconds);
    void updateChaser(sf::Vector2f playerPosition, float deltaSeconds);
    void startLaserCharge(sf::Vector2f playerPosition);
    void drawLaser(sf::RenderTarget& target, sf::Color color, float thickness);

    EnemyType type_{EnemyType::Chaser};
    std::unique_ptr<sf::Shape> body_;
    float health_{35.0f};
    float maxHealth_{35.0f};
    float moveSpeed_{90.0f};
    float shootCooldownSeconds_{1.35f};
    float shootCooldownRemaining_{0.0f};
    float laserCooldownRemaining_{1.0f};
    float laserStateTimer_{0.0f};
    int laserState_{0};
    bool laserHitPlayer_{false};
    sf::Vector2f laserDirection_{1.0f, 0.0f};
};

}
