#pragma once

#include "IRangedWeapon.hpp"

#include <optional>
#include <string_view>

namespace dungeon {

class Laser final : public IRangedWeapon {
public:
    WeaponType type() const override;
    std::string_view displayName() const override;
    void update(float deltaSeconds) override;
    void attack(sf::Vector2f origin, sf::Vector2f direction) override;
    [[nodiscard]] std::optional<Projectile> tryShoot(sf::Vector2f origin, sf::Vector2f direction) override;
    int ammo() const override;
    int magazineSize() const override;
    void reload() override;

private:
    float cooldownRemaining_{0.0f};
};

}
