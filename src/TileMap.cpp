#include "TileMap.hpp"

#include <algorithm>
#include <cstdlib>

namespace dungeon {

namespace {

constexpr int roomSize = 28;
constexpr int roomGap = 10;
constexpr int mapMargin = 2;
constexpr int corridorWidth = 8;
constexpr int maxRoomY = 2;

struct RoomRect {
    int left{};
    int top{};
    int width{};
    int height{};
};

[[nodiscard]] RoomRect roomRect(int gridX, int gridY)
{
    return {
        mapMargin + gridX * (roomSize + roomGap),
        mapMargin + (maxRoomY - gridY) * (roomSize + roomGap),
        roomSize,
        roomSize
    };
}

[[nodiscard]] sf::Vector2i roomCenterTile(int gridX, int gridY)
{
    const RoomRect room = roomRect(gridX, gridY);
    return {room.left + room.width / 2, room.top + room.height / 2};
}

}

TileMap::TileMap()
{
    generateRoom(25, 18);
}

void TileMap::generateRoom(std::size_t, std::size_t)
{
    const int mapWidth = mapMargin * 2 + 4 * roomSize + 3 * roomGap;
    const int mapHeight = mapMargin * 2 + 3 * roomSize + 2 * roomGap;
    std::vector<std::vector<int>> walkable(
        static_cast<std::size_t>(mapHeight),
        std::vector<int>(static_cast<std::size_t>(mapWidth), 0));
    tiles_.assign(static_cast<std::size_t>(mapHeight), std::vector<int>(static_cast<std::size_t>(mapWidth), 0));

    const auto carveRect = [&walkable](int left, int top, int width, int height) {
        for (int row = top; row < top + height; ++row) {
            for (int column = left; column < left + width; ++column) {
                walkable[static_cast<std::size_t>(row)][static_cast<std::size_t>(column)] = 1;
            }
        }
    };

    const auto carveRoom = [&carveRect](int gridX, int gridY) {
        const RoomRect room = roomRect(gridX, gridY);
        carveRect(room.left, room.top, room.width, room.height);
    };

    const auto carveHorizontalCorridor = [&carveRect](int leftRoomX, int roomY, int rightRoomX) {
        const sf::Vector2i leftCenter = roomCenterTile(leftRoomX, roomY);
        const sf::Vector2i rightCenter = roomCenterTile(rightRoomX, roomY);
        const int left = std::min(leftCenter.x, rightCenter.x);
        const int width = std::abs(rightCenter.x - leftCenter.x) + 1;
        carveRect(left, leftCenter.y - corridorWidth / 2, width, corridorWidth);
    };

    const auto carveVerticalCorridor = [&carveRect](int roomX, int lowerRoomY, int upperRoomY) {
        const sf::Vector2i lowerCenter = roomCenterTile(roomX, lowerRoomY);
        const sf::Vector2i upperCenter = roomCenterTile(roomX, upperRoomY);
        const int top = std::min(lowerCenter.y, upperCenter.y);
        const int height = std::abs(upperCenter.y - lowerCenter.y) + 1;
        carveRect(lowerCenter.x - corridorWidth / 2, top, corridorWidth, height);
    };

    carveRoom(0, 0);
    carveRoom(1, 0);
    carveRoom(2, 0);
    carveRoom(1, 1);
    carveRoom(2, 1);
    carveRoom(2, 2);
    carveRoom(3, 2);

    carveHorizontalCorridor(0, 0, 1);
    carveHorizontalCorridor(1, 0, 2);
    carveHorizontalCorridor(1, 1, 2);
    carveHorizontalCorridor(2, 2, 3);

    carveVerticalCorridor(1, 0, 1);
    carveVerticalCorridor(2, 0, 1);
    carveVerticalCorridor(2, 1, 2);

    for (int row = 0; row < mapHeight; ++row) {
        for (int column = 0; column < mapWidth; ++column) {
            if (walkable[static_cast<std::size_t>(row)][static_cast<std::size_t>(column)] == 1) {
                continue;
            }

            bool touchesWalkable = false;
            for (int neighborY = row - 1; neighborY <= row + 1; ++neighborY) {
                for (int neighborX = column - 1; neighborX <= column + 1; ++neighborX) {
                    if (neighborY < 0 || neighborX < 0 || neighborY >= mapHeight || neighborX >= mapWidth) {
                        continue;
                    }
                    if (walkable[static_cast<std::size_t>(neighborY)][static_cast<std::size_t>(neighborX)] == 1) {
                        touchesWalkable = true;
                    }
                }
            }

            if (touchesWalkable) {
                tiles_[static_cast<std::size_t>(row)][static_cast<std::size_t>(column)] = 1;
            }
        }
    }
}

void TileMap::render(sf::RenderTarget& target) const
{
    sf::RectangleShape tile({tileSize_, tileSize_});

    for (std::size_t row = 0; row < tiles_.size(); ++row) {
        for (std::size_t column = 0; column < tiles_[row].size(); ++column) {
            tile.setPosition(static_cast<float>(column) * tileSize_, static_cast<float>(row) * tileSize_);
            tile.setFillColor(tiles_[row][column] == 1 ? sf::Color::White : sf::Color::Black);
            tile.setOutlineThickness(0.0f);
            target.draw(tile);
        }
    }
}

bool TileMap::collides(const sf::FloatRect& bounds) const
{
    const int left = static_cast<int>(bounds.left / tileSize_);
    const int right = static_cast<int>((bounds.left + bounds.width) / tileSize_);
    const int top = static_cast<int>(bounds.top / tileSize_);
    const int bottom = static_cast<int>((bounds.top + bounds.height) / tileSize_);

    for (int row = top; row <= bottom; ++row) {
        for (int column = left; column <= right; ++column) {
            if (isWall(column, row)) {
                return true;
            }
        }
    }

    return false;
}

sf::Vector2f TileMap::spawnPosition() const
{
    const sf::Vector2i spawnTile = roomCenterTile(0, 0);
    return {
        (static_cast<float>(spawnTile.x) + 0.5f) * tileSize_,
        (static_cast<float>(spawnTile.y) + 0.5f) * tileSize_
    };
}

float TileMap::tileSize() const
{
    return tileSize_;
}

bool TileMap::isWall(int column, int row) const
{
    if (row < 0 || column < 0) {
        return true;
    }

    const auto rowIndex = static_cast<std::size_t>(row);
    const auto columnIndex = static_cast<std::size_t>(column);

    if (rowIndex >= tiles_.size() || tiles_.empty() || columnIndex >= tiles_[rowIndex].size()) {
        return true;
    }

    return tiles_[rowIndex][columnIndex] == 1;
}

}
