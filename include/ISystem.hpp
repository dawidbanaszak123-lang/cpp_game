#pragma once

namespace dungeon {

class ISystem {
public:
    virtual ~ISystem() = default;

    virtual void update(float deltaSeconds) = 0;
};

}
