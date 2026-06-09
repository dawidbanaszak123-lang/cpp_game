#pragma once

#include "GameStats.hpp"

namespace dungeon {

class IStatsTracker {
public:
    virtual ~IStatsTracker() = default;

    virtual void reset() = 0;
    virtual void recordEnemyKilled() = 0;
    virtual void recordBossKilled() = 0;
    virtual void recordRoomCleared() = 0;
    virtual void recordShotFired() = 0;
    virtual void recordMeleeAttack() = 0;
    virtual void recordDodgeRoll() = 0;
    virtual void addElapsedTime(float deltaSeconds) = 0;
    [[nodiscard]] virtual GameStats snapshot() const = 0;
};

}
