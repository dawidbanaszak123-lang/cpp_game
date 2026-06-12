#pragma once

#include "GameContext.hpp"
#include "IState.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <vector>

namespace dungeon {

enum class PostGameResult {
    Win,
    Lose
};

class PostGameStatsState final : public IState {
public:
    PostGameStatsState(GameContext& context, PostGameResult result);

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float deltaSeconds) override;
    void render(sf::RenderTarget& target) override;
    [[nodiscard]] bool allowsUnderlyingUpdate() const override;
    [[nodiscard]] bool allowsUnderlyingRender() const override;

private:
    void moveSelection(int offset);
    void chooseSelectedOption();
    void updateTextColors();
    [[nodiscard]] bool optionContains(std::size_t index, sf::Vector2f point) const;

    GameContext& context_;
    PostGameResult result_;
    sf::Font font_;
    sf::RectangleShape background_;
    sf::Text titleText_;
    std::vector<sf::Text> options_;
    std::size_t selectedIndex_{};
};

}
