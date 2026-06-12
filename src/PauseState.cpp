#include "PauseState.hpp"

#include "StateStack.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

namespace dungeon {

namespace {

constexpr const char* kSystemFontPath = "C:/Windows/Fonts/arial.ttf";

}

PauseState::PauseState(GameContext& context) : context_(context)
{
    font_.loadFromFile(kSystemFontPath);

    background_.setFillColor(sf::Color(0, 0, 0, 160));

    titleText_.setFont(font_);
    titleText_.setString("PAUSED");
    titleText_.setCharacterSize(42);
    titleText_.setFillColor(sf::Color::White);

    resumeTriangle_.setPointCount(3);
    resumeTriangle_.setPoint(0, {0.0f, 0.0f});
    resumeTriangle_.setPoint(1, {0.0f, 54.0f});
    resumeTriangle_.setPoint(2, {46.0f, 27.0f});

    exitText_.setFont(font_);
    exitText_.setString("Give up?");
    exitText_.setCharacterSize(30);

    updateTextColors();
}

void PauseState::onEnter()
{
    if (context_.window) {
        context_.window->setMouseCursorVisible(true);
    }
}

void PauseState::onExit()
{
    if (context_.window) {
        context_.window->setMouseCursorVisible(false);
    }
}

void PauseState::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
        context_.states->pop();
    } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Up) {
        selectedOption_ = 0;
        updateTextColors();
    } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Down) {
        selectedOption_ = 1;
        updateTextColors();
    } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
        chooseSelectedOption();
    } else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        const sf::Vector2f mousePosition{
            static_cast<float>(event.mouseButton.x),
            static_cast<float>(event.mouseButton.y)
        };

        if (resumeTriangle_.getGlobalBounds().contains(mousePosition)) {
            selectedOption_ = 0;
            chooseSelectedOption();
        } else if (exitText_.getGlobalBounds().contains(mousePosition)) {
            selectedOption_ = 1;
            chooseSelectedOption();
        }
    }
}

void PauseState::update(float) {}

void PauseState::render(sf::RenderTarget& target)
{
    const sf::View previousView = target.getView();
    target.setView(target.getDefaultView());

    const sf::Vector2u windowSize = context_.window->getSize();
    background_.setSize({static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)});
    background_.setPosition({0.0f, 0.0f});

    const float centerX = static_cast<float>(windowSize.x) * 0.5f;
    titleText_.setPosition({centerX - titleText_.getGlobalBounds().width * 0.5f, 170.0f});
    resumeTriangle_.setPosition({centerX - resumeTriangle_.getGlobalBounds().width * 0.5f, 245.0f});
    exitText_.setPosition({centerX - exitText_.getGlobalBounds().width * 0.5f, 315.0f});

    target.draw(background_);
    target.draw(titleText_);
    target.draw(resumeTriangle_);
    target.draw(exitText_);

    target.setView(previousView);
}

bool PauseState::allowsUnderlyingUpdate() const { return false; }
bool PauseState::allowsUnderlyingRender() const { return true; }

void PauseState::updateTextColors()
{
    resumeTriangle_.setFillColor(selectedOption_ == 0 ? sf::Color::Yellow : sf::Color::White);
    exitText_.setFillColor(selectedOption_ == 1 ? sf::Color::Yellow : sf::Color::White);
}

void PauseState::chooseSelectedOption()
{
    if (selectedOption_ == 0) {
        context_.states->pop();
    } else if (context_.returnToMainMenu) {
        context_.returnToMainMenu();
    }
}

}
