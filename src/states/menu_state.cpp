#include "states/menu_state.h"
#include "utils/constants.h"
#include <iostream>

MenuState::MenuState(StateMachine &state_machine)
    : state_machine_(state_machine), selected_index_(0)
{
}

// Load font and setup UI
void MenuState::init()
{
    if (!font_.openFromFile("assets/fonts/VT323-Regular.ttf"))
    {
        std::cerr << "Failed to load font: assets/fonts/VT323-Regular.ttf" << std::endl;
    }

    setupUI();
}

// Initialize menu UI with options
void MenuState::setupUI()
{
    std::vector<std::string> menu_options = {"START GAME", "LOAD GAME", "SETTINGS", "EXIT"};
    float initial_y = 400.0f;
    float spacing = 70.0f;

    for (size_t i = 0; i < menu_options.size(); ++i)
    {
        sf::Text option_text(font_);
        option_text.setString(menu_options[i]);
        option_text.setCharacterSize(48);

        if (static_cast<int>(i) == selected_index_)
        {
            option_text.setFillColor(sf::Color::Yellow);
        }
        else
        {
            option_text.setFillColor(sf::Color::White);
        }

        sf::FloatRect text_bounds = option_text.getLocalBounds();
        float x_position = (Constants::WINDOW_WIDTH - text_bounds.size.x) / 2.0f;
        float y_position = initial_y + (static_cast<float>(i) * spacing);

        option_text.setPosition({x_position, y_position});
        menu_texts_.push_back(std::move(option_text));
    }
}

// Handle keyboard input for menu navigation
void MenuState::handleEvent(const sf::Event &event)
{
    if (const auto *key_pressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (key_pressed->code == sf::Keyboard::Key::Up || key_pressed->code == sf::Keyboard::Key::W)
        {
            moveUp();
        }
        else if (key_pressed->code == sf::Keyboard::Key::Down || key_pressed->code == sf::Keyboard::Key::S)
        {
            moveDown();
        }
        else if (key_pressed->code == sf::Keyboard::Key::Enter)
        {
            switch (selected_index_)
            {
            case 0:
                break;
            case 1:
                break;
            case 2:
                break;
            case 3:
                state_machine_.removeState();
                break;
            default:
                break;
            }
        }
    }
}

// Update menu state (static)
void MenuState::update(float delta_time)
{
}

// Render menu background and options
void MenuState::render(sf::RenderWindow &window)
{
    window.clear(sf::Color(25, 25, 25));

    for (const auto &menu_text : menu_texts_)
    {
        window.draw(menu_text);
    }
}

// Move selection up with bounds checking
void MenuState::moveUp()
{
    if (selected_index_ - 1 >= 0)
    {
        menu_texts_[selected_index_].setFillColor(sf::Color::White);
        selected_index_--;
        menu_texts_[selected_index_].setFillColor(sf::Color::Yellow);
    }
}

// Move selection down with bounds checking
void MenuState::moveDown()
{
    if (selected_index_ + 1 < static_cast<int>(menu_texts_.size()))
    {
        menu_texts_[selected_index_].setFillColor(sf::Color::White);
        selected_index_++;
        menu_texts_[selected_index_].setFillColor(sf::Color::Yellow);
    }
}