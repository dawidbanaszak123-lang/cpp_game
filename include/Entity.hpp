#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>

namespace dungeon {

class Entity {
public:
    virtual ~Entity() = default;

    virtual void update(float deltaSeconds) = 0;
    virtual void render(sf::RenderTarget& target) = 0;

    [[nodiscard]] virtual sf::Vector2f position() const = 0;
    virtual void setPosition(sf::Vector2f position) = 0;
};

}
