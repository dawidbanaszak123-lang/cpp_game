#pragma once

#include "IMeleeWeapon.hpp"
#include "IRangedWeapon.hpp"
#include "Character.hpp"
#include "Projectile.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <memory>
#include <optional>
#include <vector>

namespace dungeon {

class Player : public Character {
public:
    Player();

    void update(float deltaSeconds) override;
    void render(sf::RenderTarget& target) override;

    [[nodiscard]] sf::Vector2f position() const override;
    void setPosition(sf::Vector2f position) override;
    void applyDamage(const Damage& damage) override;
    [[nodiscard]] bool isAlive() const override;
    [[nodiscard]] float health() const override;
    [[nodiscard]] float maxHealth() const override;

    void move(sf::Vector2f displacement);
    void dodgeRoll(sf::Vector2f direction);
    [[nodiscard]] bool hasInvincibilityFrames() const;
    void meleeAttack(sf::Vector2f target);
    void fireRangedWeapon(sf::Vector2f target);
    [[nodiscard]] std::optional<Projectile> tryFireRangedWeapon(sf::Vector2f target);
    void equipMeleeWeapon(std::unique_ptr<IMeleeWeapon> weapon);
    void addRangedWeapon(std::unique_ptr<IRangedWeapon> weapon);

    [[nodiscard]] sf::FloatRect bounds() const;
    [[nodiscard]] float movementSpeed() const;
    [[nodiscard]] bool isDodging() const;
    [[nodiscard]] bool canDodge() const;

private:
    sf::RectangleShape body_;
    sf::Vector2f lastMoveDirection_{1.0f, 0.0f};
    std::unique_ptr<IMeleeWeapon> meleeWeapon_;
    std::vector<std::unique_ptr<IRangedWeapon>> rangedWeapons_;
    float health_{100.0f};
    float maxHealth_{100.0f};
    float baseSpeed_{180.0f};
    float dodgeSpeedMultiplier_{3.0f};
    float dodgeDurationSeconds_{0.18f};
    float dodgeCooldownSeconds_{0.45f};
    float invincibilityDurationSeconds_{0.25f};
    float dodgeTimer_{};
    float dodgeCooldownTimer_{};
    float invincibilityTimer_{};
};

}
