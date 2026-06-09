#pragma once

#include "GameSnapshot.hpp"

#include <optional>
#include <string_view>

namespace dungeon {

class ISaveSystem {
public:
    virtual ~ISaveSystem() = default;

    virtual void save(std::string_view slotName, const GameSnapshot& snapshot) = 0;
    [[nodiscard]] virtual std::optional<GameSnapshot> load(std::string_view slotName) = 0;
    virtual void remove(std::string_view slotName) = 0;
};

}
