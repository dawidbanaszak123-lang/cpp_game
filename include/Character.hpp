#pragma once

#include "IDamageable.hpp"
#include "Entity.hpp"

namespace dungeon {

class Character : public Entity, public IDamageable {
public:
    [[nodiscard]] virtual float health() const = 0;
    [[nodiscard]] virtual float maxHealth() const = 0;
};

}
