#pragma once

#include "Character.hpp"

namespace dungeon {

enum class EnemyType {
    Chaser,
    Shooter,
    Tank,
    Swarmer
};

class Enemy : public Character {
public:
    [[nodiscard]] virtual EnemyType enemyType() const = 0;
    virtual void chooseBehavior(float deltaSeconds) = 0;
};

}
