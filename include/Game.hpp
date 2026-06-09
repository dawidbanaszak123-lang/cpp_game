#pragma once

#include "GameContext.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <memory>

namespace dungeon {

class StateStack;

class Game {
public:
    Game();
    ~Game();

    void run();

private:
    void processEvents();
    void update(float deltaSeconds);
    void render();
    void showMainMenu();
    void startGameplay();

    sf::RenderWindow window_;
    sf::Clock frameClock_;
    GameContext context_;
    std::unique_ptr<StateStack> states_;
};

}
