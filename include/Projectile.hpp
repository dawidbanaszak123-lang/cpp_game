#pragma once

#include "Damage.hpp"

#include <SFML/System/Vector2.hpp>

namespace dungeon {

struct Projectile {
    sf::Vector2f position{};
    sf::Vector2f velocity{};
    Damage damage{};
    float lifetimeSeconds{};
    float radius{5.0f};
};

}
