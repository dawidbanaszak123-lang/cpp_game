#pragma once

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

namespace dungeon {

class TileMap {
public:
    TileMap();

    void generateRoom(std::size_t width, std::size_t height);
    void render(sf::RenderTarget& target) const;

    [[nodiscard]] bool collides(const sf::FloatRect& bounds) const;
    [[nodiscard]] sf::Vector2f spawnPosition() const;
    [[nodiscard]] float tileSize() const;

private:
    [[nodiscard]] bool isWall(int column, int row) const;

    std::vector<std::vector<int>> tiles_;
    float tileSize_{32.0f};
};

}
