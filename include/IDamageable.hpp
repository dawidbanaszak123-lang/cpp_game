#pragma once

#include "Damage.hpp"

namespace dungeon {

class IDamageable {
public:
    virtual ~IDamageable() = default;

    virtual void applyDamage(const Damage& damage) = 0;
    [[nodiscard]] virtual bool isAlive() const = 0;
};

}
