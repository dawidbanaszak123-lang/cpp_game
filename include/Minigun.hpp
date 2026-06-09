#pragma once

#include "IRangedWeapon.hpp"

namespace dungeon {

class Minigun final : public IRangedWeapon {
public:
    WeaponType type() const override;
    std::string_view displayName() const override;
    void update(float deltaSeconds) override;
    void attack(sf::Vector2f origin, sf::Vector2f direction) override;
    int ammo() const override;
    int magazineSize() const override;
    void reload() override;
};

}
