#include "PauseState.hpp"

namespace dungeon {

PauseState::PauseState(GameContext& context) : context_(context) {}

void PauseState::onEnter() {}
void PauseState::onExit() {}
void PauseState::handleEvent(const sf::Event&) {}
void PauseState::update(float) {}
void PauseState::render(sf::RenderTarget&) {}
bool PauseState::allowsUnderlyingUpdate() const { return false; }
bool PauseState::allowsUnderlyingRender() const { return true; }

}
