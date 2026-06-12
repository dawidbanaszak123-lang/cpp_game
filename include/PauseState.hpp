#pragma once

#include "GameContext.hpp"
#include "IState.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

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
    sf::Font font_;
    sf::RectangleShape background_;
    sf::Text titleText_;
    sf::Text resumeText_;
    sf::Text exitText_;
    int selectedOption_{0};

    void updateTextColors();
    void chooseSelectedOption();
};

}
