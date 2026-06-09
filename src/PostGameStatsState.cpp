#include "PostGameStatsState.hpp"

namespace dungeon {

PostGameStatsState::PostGameStatsState(GameContext& context) : context_(context) {}

void PostGameStatsState::onEnter() {}
void PostGameStatsState::onExit() {}
void PostGameStatsState::handleEvent(const sf::Event&) {}
void PostGameStatsState::update(float) {}
void PostGameStatsState::render(sf::RenderTarget&) {}
bool PostGameStatsState::allowsUnderlyingUpdate() const { return false; }
bool PostGameStatsState::allowsUnderlyingRender() const { return false; }

}
