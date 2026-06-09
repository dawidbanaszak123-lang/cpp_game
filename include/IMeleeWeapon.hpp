#pragma once

#include "IWeapon.hpp"

namespace dungeon {

class IMeleeWeapon : public IWeapon {
public:
    virtual void setArcDegrees(float degrees) = 0;
};

}
