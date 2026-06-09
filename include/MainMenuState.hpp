#pragma once

#include "GameContext.hpp"
#include "IState.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <functional>
#include <vector>

namespace dungeon {

class MainMenuState final : public IState {
public:
    MainMenuState(GameContext& context, std::function<void()> onPlay, std::function<void()> onExit);

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float deltaSeconds) override;
    void render(sf::RenderTarget& target) override;
    [[nodiscard]] bool allowsUnderlyingUpdate() const override;
    [[nodiscard]] bool allowsUnderlyingRender() const override;

private:
    void rebuildMenuText();
    void moveSelection(int offset);
    void selectCurrentOption();

    GameContext& context_;
    std::function<void()> onPlay_;
    std::function<void()> onExit_;
    sf::Font font_;
    std::vector<sf::Text> options_;
    std::size_t selectedIndex_{};
};

}
