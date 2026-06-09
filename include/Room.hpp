#pragma once

#include <memory>
#include <vector>

namespace dungeon {

class Enemy;

class Room {
public:
    virtual ~Room() = default;

    virtual void update(float deltaSeconds) = 0;
    [[nodiscard]] virtual bool isCleared() const = 0;
    [[nodiscard]] virtual const std::vector<std::unique_ptr<Enemy>>& enemies() const = 0;
};

}
