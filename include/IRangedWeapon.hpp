#pragma once

#include "IWeapon.hpp"
#include "Projectile.hpp"

#include <optional>

namespace dungeon {

class IRangedWeapon : public IWeapon {
public:
    [[nodiscard]] virtual int ammo() const = 0;
    [[nodiscard]] virtual int magazineSize() const = 0;
    virtual void reload() = 0;
    [[nodiscard]] virtual std::optional<Projectile> tryShoot(sf::Vector2f origin, sf::Vector2f direction)
    {
        attack(origin, direction);
        return std::nullopt;
    }
};

}
