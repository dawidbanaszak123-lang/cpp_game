#pragma once

#include <SFML/Window/Event.hpp>

namespace sf {
class RenderTarget;
}

namespace dungeon {

class IState {
public:
    virtual ~IState() = default;

    virtual void onEnter() = 0;
    virtual void onExit() = 0;
    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void update(float deltaSeconds) = 0;
    virtual void render(sf::RenderTarget& target) = 0;
    virtual bool allowsUnderlyingUpdate() const = 0;
    virtual bool allowsUnderlyingRender() const = 0;
};

}
