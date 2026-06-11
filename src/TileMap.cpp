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

[[nodiscard]] sf::IntRect toIntRect(RoomRect room)
{
    return {room.left, room.top, room.width, room.height};
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
    rooms_.clear();
    doorTiles_.clear();
    activeDoorTiles_.clear();

    const auto carveRect = [&walkable](int left, int top, int width, int height) {
        for (int row = top; row < top + height; ++row) {
            for (int column = left; column < left + width; ++column) {
                walkable[static_cast<std::size_t>(row)][static_cast<std::size_t>(column)] = 1;
            }
        }
    };

    const auto carveRoom = [this, &carveRect](int gridX, int gridY) {
        const RoomRect room = roomRect(gridX, gridY);
        rooms_.push_back(toIntRect(room));
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

    const auto addVerticalDoor = [this](int column, int centerY, std::size_t roomIndex) {
        for (int row = centerY - corridorWidth / 2; row < centerY + corridorWidth / 2; ++row) {
            doorTiles_.push_back({column, row, roomIndex});
        }
    };

    const auto addHorizontalDoor = [this](int centerX, int row, std::size_t roomIndex) {
        for (int column = centerX - corridorWidth / 2; column < centerX + corridorWidth / 2; ++column) {
            doorTiles_.push_back({column, row, roomIndex});
        }
    };

    const auto addHorizontalConnectionDoors = [&addVerticalDoor](int leftRoomX, int roomY, int rightRoomX, std::size_t leftRoomIndex, std::size_t rightRoomIndex) {
        const RoomRect leftRoom = roomRect(leftRoomX, roomY);
        const RoomRect rightRoom = roomRect(rightRoomX, roomY);
        const int centerY = roomCenterTile(leftRoomX, roomY).y;
        addVerticalDoor(leftRoom.left + leftRoom.width, centerY, leftRoomIndex);
        addVerticalDoor(rightRoom.left - 1, centerY, rightRoomIndex);
    };

    const auto addVerticalConnectionDoors = [&addHorizontalDoor](int roomX, int lowerRoomY, int upperRoomY, std::size_t lowerRoomIndex, std::size_t upperRoomIndex) {
        const RoomRect lowerRoom = roomRect(roomX, lowerRoomY);
        const RoomRect upperRoom = roomRect(roomX, upperRoomY);
        const int centerX = roomCenterTile(roomX, lowerRoomY).x;
        addHorizontalDoor(centerX, lowerRoom.top - 1, lowerRoomIndex);
        addHorizontalDoor(centerX, upperRoom.top + upperRoom.height, upperRoomIndex);
    };

    addHorizontalConnectionDoors(0, 0, 1, 0, 1);
    addHorizontalConnectionDoors(1, 0, 2, 1, 2);
    addHorizontalConnectionDoors(1, 1, 2, 3, 4);
    addHorizontalConnectionDoors(2, 2, 3, 5, 6);

    addVerticalConnectionDoors(1, 0, 1, 1, 3);
    addVerticalConnectionDoors(2, 0, 1, 2, 4);
    addVerticalConnectionDoors(2, 1, 2, 4, 5);

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
            if (tiles_[row][column] == 1) {
                tile.setFillColor(sf::Color::White);
            } else if (tiles_[row][column] == 2) {
                tile.setFillColor(sf::Color::Red);
            } else {
                tile.setFillColor(sf::Color::Black);
            }
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

std::optional<std::size_t> TileMap::roomAt(sf::Vector2f position) const
{
    const int column = static_cast<int>(position.x / tileSize_);
    const int row = static_cast<int>(position.y / tileSize_);

    for (std::size_t index = 0; index < rooms_.size(); ++index) {
        if (rooms_[index].contains(column, row)) {
            return index;
        }
    }

    return std::nullopt;
}

std::optional<std::size_t> TileMap::roomContaining(const sf::FloatRect& bounds) const
{
    constexpr float edgeEpsilon = 0.001f;
    const int left = static_cast<int>(bounds.left / tileSize_);
    const int right = static_cast<int>((bounds.left + bounds.width - edgeEpsilon) / tileSize_);
    const int top = static_cast<int>(bounds.top / tileSize_);
    const int bottom = static_cast<int>((bounds.top + bounds.height - edgeEpsilon) / tileSize_);

    for (std::size_t index = 0; index < rooms_.size(); ++index) {
        const sf::IntRect& room = rooms_[index];
        if (left >= room.left && right < room.left + room.width &&
            top >= room.top && bottom < room.top + room.height) {
            return index;
        }
    }

    return std::nullopt;
}

sf::Vector2f TileMap::roomCenter(std::size_t roomIndex) const
{
    if (roomIndex >= rooms_.size()) {
        return spawnPosition();
    }

    const sf::IntRect room = rooms_[roomIndex];
    return {
        (static_cast<float>(room.left) + static_cast<float>(room.width) * 0.5f + 0.5f) * tileSize_,
        (static_cast<float>(room.top) + static_cast<float>(room.height) * 0.5f + 0.5f) * tileSize_
    };
}

std::size_t TileMap::roomCount() const
{
    return rooms_.size();
}

void TileMap::lockRoomDoors(std::size_t roomIndex)
{
    unlockDoors();

    for (const DoorTile& doorTile : doorTiles_) {
        if (doorTile.roomIndex != roomIndex) {
            continue;
        }

        const auto row = static_cast<std::size_t>(doorTile.row);
        const auto column = static_cast<std::size_t>(doorTile.column);
        if (row >= tiles_.size() || tiles_.empty() || column >= tiles_[row].size()) {
            continue;
        }

        tiles_[row][column] = 2;
        activeDoorTiles_.push_back({doorTile.column, doorTile.row});
    }
}

void TileMap::unlockDoors()
{
    for (const sf::Vector2i& tilePosition : activeDoorTiles_) {
        if (tilePosition.y < 0 || tilePosition.x < 0) {
            continue;
        }

        const auto row = static_cast<std::size_t>(tilePosition.y);
        const auto column = static_cast<std::size_t>(tilePosition.x);
        if (row >= tiles_.size() || tiles_.empty() || column >= tiles_[row].size()) {
            continue;
        }

        if (tiles_[row][column] == 2) {
            tiles_[row][column] = 0;
        }
    }

    activeDoorTiles_.clear();
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

    return tiles_[rowIndex][columnIndex] == 1 || tiles_[rowIndex][columnIndex] == 2;
}

}
