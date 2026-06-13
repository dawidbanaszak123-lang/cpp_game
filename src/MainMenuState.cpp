#include "MainMenuState.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Mouse.hpp>

namespace {

constexpr const char* kSystemFontPath = "C:/Windows/Fonts/arial.ttf";

}

namespace dungeon {

MainMenuState::MainMenuState(GameContext& context, std::function<void()> onPlay, std::function<void()> onExit)
    : context_(context)
    , onPlay_(std::move(onPlay))
    , onExit_(std::move(onExit))
{
    font_.loadFromFile(kSystemFontPath);
    rebuildMenuText();
}

void MainMenuState::onEnter() {}

void MainMenuState::onExit() {}

void MainMenuState::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::MouseMoved) {
        const sf::Vector2f mousePosition{static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y)};
        if (enterOptionContains(mousePosition)) {
            selectedIndex_ = 0;
            updateTextColors();
        } else if (leaveText_.getGlobalBounds().contains(mousePosition)) {
            selectedIndex_ = 1;
            updateTextColors();
        }
        return;
    }

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        const sf::Vector2f mousePosition{static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y)};
        if (enterOptionContains(mousePosition)) {
            selectedIndex_ = 0;
            selectCurrentOption();
        } else if (leaveText_.getGlobalBounds().contains(mousePosition)) {
            selectedIndex_ = 1;
            selectCurrentOption();
        }
        return;
    }

    if (event.type != sf::Event::KeyPressed) {
        return;
    }

    if (event.key.code == sf::Keyboard::Up) {
        moveSelection(-1);
    } else if (event.key.code == sf::Keyboard::Down) {
        moveSelection(1);
    } else if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Return) {
        selectCurrentOption();
    }
}

void MainMenuState::update(float) {}

void MainMenuState::render(sf::RenderTarget& target)
{
    target.draw(titleText_);
    target.draw(enterText_);
    target.draw(dungeonText_);
    target.draw(leaveText_);
}

bool MainMenuState::allowsUnderlyingUpdate() const
{
    return false;
}

bool MainMenuState::allowsUnderlyingRender() const
{
    return false;
}

void MainMenuState::rebuildMenuText()
{
    titleText_.setFont(font_);
    titleText_.setString("DungeonGod");
    titleText_.setCharacterSize(54);
    titleText_.setFillColor(sf::Color::White);
    titleText_.setPosition(245.0f, 135.0f);

    enterText_.setFont(font_);
    enterText_.setString("ENTER");
    enterText_.setCharacterSize(38);
    enterText_.setPosition(235.0f, 265.0f);

    dungeonText_.setFont(font_);
    dungeonText_.setString(" the dungeon");
    dungeonText_.setCharacterSize(38);
    dungeonText_.setPosition(enterText_.getPosition().x + enterText_.getGlobalBounds().width, 265.0f);

    leaveText_.setFont(font_);
    leaveText_.setString("leave...");
    leaveText_.setCharacterSize(34);
    leaveText_.setPosition(335.0f, 340.0f);

    updateTextColors();
}

void MainMenuState::moveSelection(int offset)
{
    const int size = 2;
    selectedIndex_ = static_cast<std::size_t>((static_cast<int>(selectedIndex_) + offset + size) % size);
    updateTextColors();
}

void MainMenuState::selectCurrentOption()
{
    if (selectedIndex_ == 0) {
        onPlay_();
    } else if (selectedIndex_ == 1) {
        onExit_();
    }
}

void MainMenuState::updateTextColors()
{
    enterText_.setFillColor(selectedIndex_ == 0 ? sf::Color(255, 220, 80) : sf::Color::White);
    dungeonText_.setFillColor(sf::Color::White);
    leaveText_.setFillColor(selectedIndex_ == 1 ? sf::Color(180, 180, 180) : sf::Color::White);
}

bool MainMenuState::enterOptionContains(sf::Vector2f point) const
{
    return enterText_.getGlobalBounds().contains(point) || dungeonText_.getGlobalBounds().contains(point);
}

}
