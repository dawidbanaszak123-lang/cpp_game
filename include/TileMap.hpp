#pragma once

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <optional>
#include <vector>

namespace dungeon {

class TileMap {
public:
    TileMap();

    void generateRoom(std::size_t width, std::size_t height);
    void render(sf::RenderTarget& target) const;

    [[nodiscard]] bool collides(const sf::FloatRect& bounds) const;
    [[nodiscard]] sf::Vector2f spawnPosition() const;
    [[nodiscard]] std::optional<std::size_t> roomAt(sf::Vector2f position) const;
    [[nodiscard]] std::optional<std::size_t> roomContaining(const sf::FloatRect& bounds) const;
    [[nodiscard]] sf::Vector2f roomCenter(std::size_t roomIndex) const;
    [[nodiscard]] std::size_t roomCount() const;
    void lockRoomDoors(std::size_t roomIndex);
    void unlockDoors();
    [[nodiscard]] float tileSize() const;

private:
    struct DoorTile {
        int column{};
        int row{};
        std::size_t roomIndex{};
    };

    [[nodiscard]] bool isWall(int column, int row) const;

    std::vector<std::vector<int>> tiles_;
    std::vector<sf::IntRect> rooms_;
    std::vector<DoorTile> doorTiles_;
    std::vector<sf::Vector2i> activeDoorTiles_;
    float tileSize_{32.0f};
};

}
