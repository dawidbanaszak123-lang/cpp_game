#include "PostGameStatsState.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

namespace dungeon {

namespace {

constexpr const char* kSystemFontPath = "C:/Windows/Fonts/arial.ttf";

}

PostGameStatsState::PostGameStatsState(GameContext& context, PostGameResult result)
    : context_(context)
    , result_(result)
{
    font_.loadFromFile(kSystemFontPath);

    background_.setFillColor(sf::Color(0, 0, 0, 235));

    titleText_.setFont(font_);
    titleText_.setString(result_ == PostGameResult::Win ? "DUNGEON CLEARED" : "RIP");
    titleText_.setCharacterSize(result_ == PostGameResult::Win ? 58 : 86);
    titleText_.setFillColor(result_ == PostGameResult::Win ? sf::Color(255, 220, 80) : sf::Color(230, 45, 45));

    const std::vector<const char*> labels = result_ == PostGameResult::Win
        ? std::vector<const char*>{"back to menu"}
        : std::vector<const char*>{"retry", "give up"};

    for (const char* label : labels) {
        sf::Text option;
        option.setFont(font_);
        option.setString(label);
        option.setCharacterSize(34);
        options_.push_back(option);
    }

    updateTextColors();
}

void PostGameStatsState::onEnter()
{
    if (context_.window) {
        context_.window->setMouseCursorVisible(true);
    }
}

void PostGameStatsState::onExit() {}

void PostGameStatsState::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::MouseMoved) {
        const sf::Vector2f mousePosition{static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y)};
        for (std::size_t index = 0; index < options_.size(); ++index) {
            if (optionContains(index, mousePosition)) {
                selectedIndex_ = index;
                updateTextColors();
                return;
            }
        }
    } else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        const sf::Vector2f mousePosition{static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y)};
        for (std::size_t index = 0; index < options_.size(); ++index) {
            if (optionContains(index, mousePosition)) {
                selectedIndex_ = index;
                chooseSelectedOption();
                return;
            }
        }
    } else if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Up) {
            moveSelection(-1);
        } else if (event.key.code == sf::Keyboard::Down) {
            moveSelection(1);
        } else if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Return) {
            chooseSelectedOption();
        }
    }
}

void PostGameStatsState::update(float) {}

void PostGameStatsState::render(sf::RenderTarget& target)
{
    const sf::View previousView = target.getView();
    target.setView(target.getDefaultView());

    const sf::Vector2u windowSize = context_.window->getSize();
    background_.setSize({static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)});

    const float centerX = static_cast<float>(windowSize.x) * 0.5f;
    titleText_.setPosition({centerX - titleText_.getGlobalBounds().width * 0.5f, 170.0f});

    const float startY = result_ == PostGameResult::Win ? 315.0f : 330.0f;
    for (std::size_t index = 0; index < options_.size(); ++index) {
        sf::Text& option = options_[index];
        option.setPosition({centerX - option.getGlobalBounds().width * 0.5f, startY + static_cast<float>(index) * 58.0f});
    }

    target.draw(background_);
    target.draw(titleText_);
    for (const auto& option : options_) {
        target.draw(option);
    }

    target.setView(previousView);
}

bool PostGameStatsState::allowsUnderlyingUpdate() const { return false; }
bool PostGameStatsState::allowsUnderlyingRender() const { return false; }

void PostGameStatsState::moveSelection(int offset)
{
    if (options_.empty()) {
        return;
    }

    const int size = static_cast<int>(options_.size());
    selectedIndex_ = static_cast<std::size_t>((static_cast<int>(selectedIndex_) + offset + size) % size);
    updateTextColors();
}

void PostGameStatsState::chooseSelectedOption()
{
    if (result_ == PostGameResult::Lose && selectedIndex_ == 0) {
        if (context_.restartGameplay) {
            context_.restartGameplay();
        }
        return;
    }

    if (context_.returnToMainMenu) {
        context_.returnToMainMenu();
    }
}

void PostGameStatsState::updateTextColors()
{
    for (std::size_t index = 0; index < options_.size(); ++index) {
        options_[index].setFillColor(index == selectedIndex_ ? sf::Color(255, 220, 80) : sf::Color::White);
    }
}

bool PostGameStatsState::optionContains(std::size_t index, sf::Vector2f point) const
{
    return index < options_.size() && options_[index].getGlobalBounds().contains(point);
}

}
