#include "TileMap.hpp"

#include <algorithm>

namespace dungeon {

TileMap::TileMap()
{
    generateRoom(25, 18);
}

void TileMap::generateRoom(std::size_t width, std::size_t height)
{
    tiles_.assign(height, std::vector<int>(width, 0));

    for (std::size_t row = 0; row < height; ++row) {
        for (std::size_t column = 0; column < width; ++column) {
            const bool isBoundary = row == 0 || column == 0 || row + 1 == height || column + 1 == width;
            tiles_[row][column] = isBoundary ? 1 : 0;
        }
    }

    if (height > 10 && width > 14) {
        for (std::size_t column = 6; column < 14; ++column) {
            tiles_[8][column] = 1;
        }
        tiles_[8][9] = 0;
    }
}

void TileMap::render(sf::RenderTarget& target) const
{
    sf::RectangleShape tile({tileSize_, tileSize_});

    for (std::size_t row = 0; row < tiles_.size(); ++row) {
        for (std::size_t column = 0; column < tiles_[row].size(); ++column) {
            tile.setPosition(static_cast<float>(column) * tileSize_, static_cast<float>(row) * tileSize_);
            tile.setFillColor(tiles_[row][column] == 1 ? sf::Color(70, 72, 82) : sf::Color(34, 36, 42));
            tile.setOutlineThickness(1.0f);
            tile.setOutlineColor(sf::Color(24, 24, 28));
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
    return {tileSize_ * 2.5f, tileSize_ * 2.5f};
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
