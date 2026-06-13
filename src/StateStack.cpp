#include "StateStack.hpp"
#include <algorithm>

namespace dungeon {

void StateStack::push(StatePtr state)
{
    if (state) {
        state->onEnter();
        states_.push_back(std::move(state));
    }
}

void StateStack::pop()
{
    if (states_.empty()) {
        return;
    }

    states_.back()->onExit();
    states_.pop_back();
}

void StateStack::clear()
{
    while (!states_.empty()) {
        pop();
    }
}

void StateStack::handleEvent(const sf::Event& event)
{
    if (!states_.empty()) {
        states_.back()->handleEvent(event);
    }
}

void StateStack::update(float deltaSeconds)
{
    for (auto it = states_.rbegin(); it != states_.rend(); ++it) {
        (*it)->update(deltaSeconds);
        if (!(*it)->allowsUnderlyingUpdate()) {
            break;
        }
    }
}

void StateStack::render(sf::RenderTarget& target)
{
    auto firstVisible = states_.begin();
    for (auto it = states_.rbegin(); it != states_.rend(); ++it) {
        firstVisible = std::prev(it.base());
        if (!(*it)->allowsUnderlyingRender()) {
            break;
        }
    }

    for (auto it = firstVisible; it != states_.end(); ++it) {
        (*it)->render(target);
    }
}

bool StateStack::empty() const
{
    return states_.empty();
}

}
