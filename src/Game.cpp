#include "Game.hpp"

#include "GameplayState.hpp"
#include "MainMenuState.hpp"
#include "StateStack.hpp"

#include <SFML/Window/Event.hpp>

namespace dungeon {

Game::Game()
    : window_(sf::VideoMode(800, 600), "DungeonGod MVP")
    , states_(std::make_unique<StateStack>())
{
    window_.setFramerateLimit(60);
    context_.window = &window_;
    context_.states = states_.get();
    context_.returnToMainMenu = [this]() { returnToMainMenuRequested_ = true; };
    showMainMenu();
}

Game::~Game() = default;

void Game::run()
{
    while (window_.isOpen()) {
        const float deltaSeconds = frameClock_.restart().asSeconds();
        processEvents();
        update(deltaSeconds);
        render();
    }
}

void Game::processEvents()
{
    sf::Event event{};
    while (window_.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window_.close();
            continue;
        }

        if (states_) {
            states_->handleEvent(event);
        }
    }
}

void Game::update(float deltaSeconds)
{
    if (states_) {
        states_->update(deltaSeconds);
    }

    if (returnToMainMenuRequested_) {
        returnToMainMenuRequested_ = false;
        showMainMenu();
    }
}

void Game::render()
{
    window_.clear(sf::Color::Black);
    if (states_) {
        states_->render(window_);
    }
    window_.display();
}

void Game::showMainMenu()
{
    states_->clear();
    states_->push(std::make_unique<MainMenuState>(
        context_,
        [this]() { startGameplay(); },
        [this]() { window_.close(); }));
}

void Game::startGameplay()
{
    states_->clear();
    states_->push(std::make_unique<GameplayState>(context_));
}

}
