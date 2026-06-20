#include "states/playing_state.h"
#include "utils/constants.h"
#include <iostream>

PlayingState::PlayingState(StateMachine &state_machine) : state_machine_(state_machine), score_(0) {}

// Initialize game UI and player position
void PlayingState::init()
{
    std::cout << "[PlayingState] Initializing...\n";

    if (!font_.openFromFile("assets/fonts/VT323-Regular.ttf"))
    {
        std::cerr << "Error: Cannot load font in PlayingState!\n";
    }

    score_text_ = std::make_unique<sf::Text>(font_);
    score_text_->setString("Score: 0");
    score_text_->setCharacterSize(40);
    score_text_->setFillColor(sf::Color::White);
    score_text_->setOutlineColor(sf::Color::Black);
    score_text_->setOutlineThickness(2.f);
    score_text_->setPosition({20.f, 20.f});

    player_.setSize({50.f, 50.f});
    player_.setFillColor(sf::Color::Green);
    player_.setOrigin({25.f, 25.f});

    player_.setPosition({Constants::WINDOW_WIDTH / 2.f, Constants::WINDOW_HEIGHT - 100.f});
}

// Handle player movement input
void PlayingState::handleEvent(const sf::Event &event)
{
    if (const auto *key_pressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (key_pressed->code == sf::Keyboard::Key::P || key_pressed->code == sf::Keyboard::Key::Escape)
        {
            std::cout << "P pressed -> Transition to Pause Menu\n";
        }

        float step = 50.f;

        if (key_pressed->code == sf::Keyboard::Key::W || key_pressed->code == sf::Keyboard::Key::Up)
        {
            player_.move({0.f, -step});
            score_++;
            score_text_->setString("Score: " + std::to_string(score_));
        }
        else if (key_pressed->code == sf::Keyboard::Key::S || key_pressed->code == sf::Keyboard::Key::Down)
        {
            player_.move({0.f, step});
        }
        else if (key_pressed->code == sf::Keyboard::Key::A || key_pressed->code == sf::Keyboard::Key::Left)
        {
            player_.move({-step, 0.f});
        }
        else if (key_pressed->code == sf::Keyboard::Key::D || key_pressed->code == sf::Keyboard::Key::Right)
        {
            player_.move({step, 0.f});
        }
    }
}

// Update game logic and collision detection
void PlayingState::update(float dt)
{
}

// Render game scene with player and score
void PlayingState::render(sf::RenderWindow &window)
{
    window.clear(sf::Color(60, 60, 60));
    window.draw(player_);

    if (score_text_)
    {
        window.draw(*score_text_);
    }
}

// Called when pause menu is opened
void PlayingState::pause()
{
    std::cout << "[PlayingState] Paused\n";
}

// Called when pause menu is closed
void PlayingState::resume()
{
    std::cout << "[PlayingState] Resumed\n";
}