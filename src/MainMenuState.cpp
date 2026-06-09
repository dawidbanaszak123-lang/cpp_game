#include "MainMenuState.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

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
    for (const auto& option : options_) {
        target.draw(option);
    }
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
    options_.clear();

    const std::vector<const char*> labels{"Play", "Load Game", "Exit"};
    const float startY = 220.0f;

    for (std::size_t i = 0; i < labels.size(); ++i) {
        sf::Text text;
        text.setFont(font_);
        text.setString(labels[i]);
        text.setCharacterSize(36);
        text.setPosition(330.0f, startY + static_cast<float>(i) * 58.0f);
        options_.push_back(text);
    }

    moveSelection(0);
}

void MainMenuState::moveSelection(int offset)
{
    if (options_.empty()) {
        return;
    }

    const auto size = static_cast<int>(options_.size());
    selectedIndex_ = static_cast<std::size_t>((static_cast<int>(selectedIndex_) + offset + size) % size);

    for (std::size_t i = 0; i < options_.size(); ++i) {
        options_[i].setFillColor(i == selectedIndex_ ? sf::Color(255, 210, 80) : sf::Color::White);
    }
}

void MainMenuState::selectCurrentOption()
{
    if (selectedIndex_ == 0) {
        onPlay_();
    } else if (selectedIndex_ == 1) {
        onPlay_();
    } else if (selectedIndex_ == 2) {
        onExit_();
    }
}

}
