#pragma once

#include "GameContext.hpp"
#include "Enemy.hpp"
#include "Player.hpp"
#include "IState.hpp"
#include "Projectile.hpp"
#include "TileMap.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/View.hpp>
#include <cstddef>
#include <optional>
#include <vector>

namespace dungeon {

class GameplayState final : public IState {
public:
    explicit GameplayState(GameContext& context);

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float deltaSeconds) override;
    void render(sf::RenderTarget& target) override;
    [[nodiscard]] bool allowsUnderlyingUpdate() const override;
    [[nodiscard]] bool allowsUnderlyingRender() const override;

private:
    struct RoomEncounter {
        bool visited{};
        bool cleared{};
    };

    void updatePlayer(float deltaSeconds);
    void updateRoomEncounters();
    void spawnRoomEnemies(std::size_t roomIndex);
    void updateEnemies(float deltaSeconds);
    void updateProjectiles(float deltaSeconds);
    void fireProjectile(sf::Vector2f target);
    void renderHud(sf::RenderTarget& target);
    void renderCrosshair(sf::RenderTarget& target);
    [[nodiscard]] sf::View cameraView() const;
    [[nodiscard]] sf::Vector2f readMovementInput() const;

    GameContext& context_;
    TileMap map_;
    Player player_;
    std::optional<std::size_t> currentRoomIndex_;
    std::vector<RoomEncounter> roomEncounters_;
    std::vector<Enemy> enemies_;
    std::vector<Projectile> projectiles_;
    sf::CircleShape projectileShape_;
    sf::RectangleShape crosshairHorizontal_;
    sf::RectangleShape crosshairVertical_;
    sf::Font hudFont_;
    sf::Text hpText_;
};

}
