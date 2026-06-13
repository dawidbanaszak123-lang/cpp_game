#pragma once

#include "GameContext.hpp"
#include "Enemy.hpp"
#include "Player.hpp"
#include "IState.hpp"
#include "PostGameStatsState.hpp"
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
    // Tworzy główny stan rozgrywki.
    explicit GameplayState(GameContext& context);

    // Przygotowanie ekranu po wejściu do gry.
    void onEnter() override;
    // Przywrócenie ustawień po wyjściu z gry.
    void onExit() override;
    // Obsługa zdarzeń z klawiatury i myszy.
    void handleEvent(const sf::Event& event) override;
    // Aktualizacja całej rozgrywki.
    void update(float deltaSeconds) override;
    // Rysowanie mapy, gracza, przeciwników i interfejsu.
    void render(sf::RenderTarget& target) override;
    // Blokuje aktualizację stanów pod spodem.
    [[nodiscard]] bool allowsUnderlyingUpdate() const override;
    // Blokuje rysowanie stanów pod spodem.
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

    enum class BossAttackState {
        Cooldown,
        RangedBurst,
        Dash,
        LaserWarning,
        LaserBeam
    };

    struct FinalBoss {
        sf::CircleShape body;
        BossAttackState state{BossAttackState::Cooldown};
        float health{500.0f};
        float maxHealth{500.0f};
        float stateTimer{1.0f};
        float shootTimer{0.0f};
        bool dashHitPlayer{false};
        bool laserHitPlayer{false};
        sf::Vector2f dashTarget{};
        sf::Vector2f dashDirection{};
        sf::Vector2f laserDirection{1.0f, 0.0f};
    };

    // Aktualizacja ruchu gracza i kolizji z mapą.
    void updatePlayer(float deltaSeconds);
    // Sprawdzenie, czy gracz wszedł do nowego pokoju.
    void updateRoomEncounters();
    // Tworzenie przeciwników w wybranym pokoju.
    void spawnRoomEnemies(std::size_t roomIndex);
    // Dodanie kilku przeciwników danego typu.
    void addEnemies(EnemyType type, int count, std::size_t roomIndex);
    // Losowanie bezpiecznego miejsca pojawienia się przeciwnika.
    [[nodiscard]] sf::Vector2f randomSpawnPosition(std::size_t roomIndex);
    // Aktualizacja przeciwników i ich ataków.
    void updateEnemies(float deltaSeconds);
    // Tworzenie finałowego bossa w ostatnim pokoju.
    void spawnFinalBoss(std::size_t roomIndex);
    // Aktualizacja zachowania finałowego bossa.
    void updateFinalBoss(float deltaSeconds);
    // Wybór następnego ataku bossa.
    void chooseFinalBossAttack();
    // Rysowanie ostrzeżenia lub promienia lasera bossa.
    void drawFinalBossLaser(sf::RenderTarget& target);
    // Rysowanie paska życia bossa.
    void renderBossHealthBar(sf::RenderTarget& target);
    // Sprawdzenie, czy pokój jest ostatni.
    [[nodiscard]] bool isFinalRoom(std::size_t roomIndex) const;
    // Pobranie obszaru kolizji bossa.
    [[nodiscard]] sf::FloatRect bossBounds() const;
    // Zadanie obrażeń bossowi.
    void damageBoss(const Damage& damage);
    // Zgłoszenie końca gry.
    void requestEndGame(PostGameResult result);
    // Pokazanie ekranu końcowego, jeśli jest potrzebny.
    void showPendingEndGame();
    // Aktualizacja pocisków i ich kolizji.
    void updateProjectiles(float deltaSeconds);
    // Aktualizacja efektów wybuchów i laserów.
    void updateEffects(float deltaSeconds);
    // Aktualizacja fal przeciwników.
    void updateWaves(float deltaSeconds);
    // Obsługa podnoszenia broni z ziemi.
    void updateWeaponPickup();
    // Wystrzelenie pocisku w stronę celu.
    void fireProjectile(sf::Vector2f target);
    // Rozpoczęcie wybranej fali przeciwników.
    void startWave(int waveNumber);
    // Wylosowanie broni po oczyszczeniu pokoju.
    void spawnRandomWeapon();
    // Utworzenie efektu wybuchu.
    void createExplosion(sf::Vector2f position);
    // Obsługa strzału laserowego gracza.
    void fireLaser(const Projectile& laserShot);
    // Utworzenie broni danego typu.
    [[nodiscard]] std::unique_ptr<IRangedWeapon> makeWeapon(WeaponType type) const;
    // Rysowanie informacji o życiu, broni i fali.
    void renderHud(sf::RenderTarget& target);
    // Rysowanie celownika myszy.
    void renderCrosshair(sf::RenderTarget& target);
    // Utworzenie widoku kamery śledzącej gracza.
    [[nodiscard]] sf::View cameraView() const;
    // Odczyt kierunku ruchu z klawiatury.
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
    std::optional<FinalBoss> finalBoss_;
    sf::CircleShape projectileShape_;
    sf::RectangleShape crosshairHorizontal_;
    sf::RectangleShape crosshairVertical_;
    sf::Font hudFont_;
    sf::Text hpText_;
    sf::Text waveText_;
    sf::Text weaponSpawnText_;
    int currentWave_{0};
    bool waitingForWeaponPickup_{false};
    bool endScreenShown_{false};
    float waveTextTimer_{0.0f};
    float weaponSpawnTextTimer_{0.0f};
    std::optional<PostGameResult> pendingEndGame_;
};

}
