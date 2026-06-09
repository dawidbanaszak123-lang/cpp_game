#pragma once

#include "InputAction.hpp"

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <unordered_map>

namespace dungeon {

class InputMapper {
public:
    void bindKey(InputAction action, sf::Keyboard::Key key);
    [[nodiscard]] bool isActionPressed(InputAction action) const;
    [[nodiscard]] bool eventMatches(InputAction action, const sf::Event& event) const;

private:
    std::unordered_map<InputAction, sf::Keyboard::Key> keyBindings_;
};

}
