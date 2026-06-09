#pragma once

#include "GameContext.hpp"
#include "IState.hpp"

namespace dungeon {

class PauseState final : public IState {
public:
    explicit PauseState(GameContext& context);

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
