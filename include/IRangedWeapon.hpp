#pragma once

#include "IWeapon.hpp"

namespace dungeon {

class IRangedWeapon : public IWeapon {
public:
    [[nodiscard]] virtual int ammo() const = 0;
    [[nodiscard]] virtual int magazineSize() const = 0;
    virtual void reload() = 0;
};

}
