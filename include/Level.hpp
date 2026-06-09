#pragma once

namespace dungeon {

class Boss;
class Room;

class Level {
public:
    virtual ~Level() = default;

    virtual void update(float deltaSeconds) = 0;
    [[nodiscard]] virtual Room* currentRoom() = 0;
    [[nodiscard]] virtual Boss* endBoss() = 0;
    [[nodiscard]] virtual bool isCompleted() const = 0;
};

}
