#include "StateStack.hpp"
#include <algorithm>

namespace dungeon {

// Dodanie nowego stanu na wierzch stosu.
void StateStack::push(StatePtr state)
{
    if (state) {
        state->onEnter();
        states_.push_back(std::move(state));
    }
}

// Usunięcie aktywnego stanu ze stosu.
void StateStack::pop()
{
    if (states_.empty()) {
        return;
    }

    states_.back()->onExit();
    states_.pop_back();
}

// Czyszczenie stosu przez poprawne zamykanie stanów.
void StateStack::clear()
{
    while (!states_.empty()) {
        pop();
    }
}

// Przekazanie zdarzenia tylko do aktywnego stanu.
void StateStack::handleEvent(const sf::Event& event)
{
    if (!states_.empty()) {
        states_.back()->handleEvent(event);
    }
}

// Aktualizacja stanów od góry stosu do pierwszej blokady.
void StateStack::update(float deltaSeconds)
{
    for (auto it = states_.rbegin(); it != states_.rend(); ++it) {
        (*it)->update(deltaSeconds);
        if (!(*it)->allowsUnderlyingUpdate()) {
            break;
        }
    }
}

// Rysowanie widocznych stanów w poprawnej kolejności.
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

// Sprawdzenie, czy stos nie zawiera stanów.
bool StateStack::empty() const
{
    return states_.empty();
}

}
