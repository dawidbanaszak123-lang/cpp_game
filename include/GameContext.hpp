#pragma once

#include <functional>

namespace sf {
class RenderWindow;
}

namespace dungeon {

class InputMapper;
class ISaveSystem;
class IStatsTracker;
class StateStack;

struct GameContext {
    sf::RenderWindow* window{};
    InputMapper* input{};
    ISaveSystem* saveSystem{};
    IStatsTracker* statsTracker{};
    StateStack* states{};
    std::function<void()> returnToMainMenu;
    std::function<void()> restartGameplay;
    std::function<void(bool)> showEndGame;
};

}
