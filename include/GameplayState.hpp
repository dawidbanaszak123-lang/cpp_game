#pragma once

#include "GameContext.hpp"
#include "Player.hpp"
#include "IState.hpp"
#include "Projectile.hpp"
#include "TileMap.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
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
    void updatePlayer(float deltaSeconds);
    void updateProjectiles(float deltaSeconds);
    void fireProjectile(sf::Vector2f target);
    void setupTargetEntities();
    [[nodiscard]] sf::Vector2f readMovementInput() const;
    [[nodiscard]] bool projectileHitsEntity(const sf::FloatRect& bounds) const;

    GameContext& context_;
    TileMap map_;
    Player player_;
    std::vector<Projectile> projectiles_;
    std::vector<sf::RectangleShape> targetEntities_;
    sf::CircleShape projectileShape_;
};

}
