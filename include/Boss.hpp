#pragma once

#include "Enemy.hpp"

#include <string_view>

namespace dungeon {

class Boss : public Enemy {
public:
    [[nodiscard]] virtual std::string_view bossName() const = 0;
    virtual void enterPhase(int phaseIndex) = 0;
    [[nodiscard]] virtual bool isLevelEndBoss() const = 0;
};

}
