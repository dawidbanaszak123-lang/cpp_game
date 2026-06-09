#pragma once

#include <cstdint>
#include <string>

namespace dungeon {

struct PlayerSnapshot {
    float x{};
    float y{};
    float health{};
    std::string equippedWeaponId;
};

struct LevelSnapshot {
    std::uint32_t levelIndex{};
    std::uint32_t roomIndex{};
};

struct GameSnapshot {
    PlayerSnapshot player;
    LevelSnapshot level;
    std::uint64_t elapsedMilliseconds{};
    std::uint32_t enemiesKilled{};
};

}
