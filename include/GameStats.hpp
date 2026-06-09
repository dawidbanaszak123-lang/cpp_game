#pragma once

#include <cstdint>

namespace dungeon {

struct GameStats {
    std::uint32_t enemiesKilled{};
    std::uint32_t bossesKilled{};
    std::uint32_t roomsCleared{};
    std::uint32_t shotsFired{};
    std::uint32_t meleeAttacksUsed{};
    std::uint32_t dodgeRollsUsed{};
    std::uint64_t elapsedMilliseconds{};
};

}
