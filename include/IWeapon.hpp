#pragma once

#include "WeaponType.hpp"

#include <SFML/System/Vector2.hpp>
#include <string_view>

namespace dungeon {

class IWeapon {
public:
    virtual ~IWeapon() = default;

    [[nodiscard]] virtual WeaponType type() const = 0;
    [[nodiscard]] virtual std::string_view displayName() const = 0;
    virtual void update(float deltaSeconds) = 0;
    virtual void attack(sf::Vector2f origin, sf::Vector2f direction) = 0;
};

}
