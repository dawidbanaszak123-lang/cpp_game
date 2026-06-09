#pragma once

#include "IState.hpp"

#include <memory>
#include <vector>

namespace dungeon {

class StateStack {
public:
    using StatePtr = std::unique_ptr<IState>;

    void push(StatePtr state);
    void pop();
    void clear();

    void handleEvent(const sf::Event& event);
    void update(float deltaSeconds);
    void render(sf::RenderTarget& target);

    [[nodiscard]] bool empty() const;

private:
    std::vector<StatePtr> states_;
};

}
