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
#include <memory>
#include <optional>
#include <string>
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

    struct ExplosionLine {
        sf::RectangleShape shape;
        sf::Vector2f velocity{};
        float lifetimeSeconds{};
        bool dealtDamage{};
    };

    struct ExplosionEffect {
        sf::CircleShape circle;
        float lifetimeSeconds{};
        float maxLifetimeSeconds{};
        std::vector<ExplosionLine> lines;
    };

    struct LaserEffect {
        sf::RectangleShape line;
        float lifetimeSeconds{};
    };

    struct WeaponPickup {
        WeaponType type{WeaponType::Rifle};
        sf::RectangleShape shape;
        std::string name;
    };

    void updatePlayer(float deltaSeconds);
    void updateRoomEncounters();
    void spawnRoomEnemies(std::size_t roomIndex);
    void addEnemies(EnemyType type, int count, std::size_t roomIndex);
    [[nodiscard]] sf::Vector2f randomSpawnPosition(std::size_t roomIndex);
    void updateEnemies(float deltaSeconds);
    void updateProjectiles(float deltaSeconds);
    void updateEffects(float deltaSeconds);
    void updateWaves(float deltaSeconds);
    void updateWeaponPickup();
    void fireProjectile(sf::Vector2f target);
    void startWave(int waveNumber);
    void spawnRandomWeapon();
    void createExplosion(sf::Vector2f position);
    void fireLaser(const Projectile& laserShot);
    [[nodiscard]] std::unique_ptr<IRangedWeapon> makeWeapon(WeaponType type) const;
    void renderHud(sf::RenderTarget& target);
    void renderCrosshair(sf::RenderTarget& target);
    [[nodiscard]] sf::View cameraView() const;
    [[nodiscard]] sf::Vector2f readMovementInput() const;

    GameContext& context_;
    TileMap map_;
    Player player_;
    std::optional<std::size_t> startingRoomIndex_;
    std::optional<std::size_t> currentRoomIndex_;
    std::vector<RoomEncounter> roomEncounters_;
    std::vector<Enemy> enemies_;
    std::vector<Projectile> projectiles_;
    std::vector<ExplosionEffect> explosions_;
    std::vector<LaserEffect> laserEffects_;
    std::optional<WeaponPickup> weaponPickup_;
    sf::CircleShape projectileShape_;
    sf::RectangleShape crosshairHorizontal_;
    sf::RectangleShape crosshairVertical_;
    sf::Font hudFont_;
    sf::Text hpText_;
    sf::Text waveText_;
    sf::Text weaponSpawnText_;
    int currentWave_{0};
    bool waitingForWeaponPickup_{false};
    float waveTextTimer_{0.0f};
    float weaponSpawnTextTimer_{0.0f};
};

}
