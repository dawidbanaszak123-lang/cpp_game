#pragma once

#include "GameContext.hpp"
#include "IState.hpp"

namespace dungeon {

class PostGameStatsState final : public IState {
public:
    explicit PostGameStatsState(GameContext& context);

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float deltaSeconds) override;
    void render(sf::RenderTarget& target) override;
    [[nodiscard]] bool allowsUnderlyingUpdate() const override;
    [[nodiscard]] bool allowsUnderlyingRender() const override;

private:
    GameContext& context_;
};

}
