#include "states/pause_state.h"
#include "states/home_state.h"
#include "states/save_state.h"
#include "states/setting_state.h"
#include "utils/constants.h"
#include <iostream>

PauseState::PauseState(StateMachine &state_machine)
    : state_machine_(state_machine), title_text_(font_), selected_index_(0)
{
}

// Load font and initialize UI
void PauseState::init()
{
    if (!font_.openFromFile("assets/fonts/VT323-Regular.ttf"))
    {
        std::cerr << "Failed to load font: assets/fonts/VT323-Regular.ttf" << std::endl;
    }

    setupUI();
}

// Setup pause menu UI with overlay and options
void PauseState::setupUI()
{
    background_overlay_.setSize({static_cast<float>(Constants::WINDOW_WIDTH),
                                 static_cast<float>(Constants::WINDOW_HEIGHT)});
    background_overlay_.setFillColor(sf::Color(0, 0, 0, 150));

    title_text_.setString("PAUSED");
    title_text_.setCharacterSize(90);
    title_text_.setFillColor(sf::Color::White);

    sf::FloatRect title_bounds = title_text_.getLocalBounds();
    float title_x = (Constants::WINDOW_WIDTH - title_bounds.size.x) / 2.0f;
    title_text_.setPosition({title_x, 200.0f});

    std::vector<std::string> options = {"RESUME", "SAVE GAME", "SETTINGS", "HOME"};
    float initial_y = 450.0f;
    float spacing = 80.0f;

    for (size_t i = 0; i < options.size(); ++i)
    {
        sf::Text option_text(font_);
        option_text.setString(options[i]);
        option_text.setCharacterSize(50);

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

// Handle pause menu input
void PauseState::handleEvent(const sf::Event &event)
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
            if (selected_index_ == 0)
            {
                state_machine_.removeState(); // nhánh 'select continue'
            }
            else if (selected_index_ == 1)
            {
                // 'select save'
                state_machine_.addState(std::make_unique<SaveState>(state_machine_), false);
            }
            else if (selected_index_ == 2)
            {
                // 'select setting'
                state_machine_.addState(std::make_unique<SettingState>(state_machine_), false);
            }
            else if (selected_index_ == 3)
            {
                // 'select home'
                state_machine_.replaceRootState(std::make_unique<HomeState>(state_machine_));
            }
        }
        else if (key_pressed->code == sf::Keyboard::Key::Escape)
        {
            state_machine_.removeState();
        }
    }
}

// Update pause state (static)
void PauseState::update(float delta_time)
{
}

// Render pause menu overlay and options
void PauseState::render(sf::RenderWindow &window)
{
    window.draw(background_overlay_);
    window.draw(title_text_);
    for (const auto &menu_text : menu_texts_)
    {
        window.draw(menu_text);
    }
}

// Move selection up
void PauseState::moveUp()
{
    if (selected_index_ - 1 >= 0)
    {
        menu_texts_[selected_index_].setFillColor(sf::Color::White);
        selected_index_--;
        menu_texts_[selected_index_].setFillColor(sf::Color::Yellow);
    }
}

// Move selection down
void PauseState::moveDown()
{
    if (selected_index_ + 1 < static_cast<int>(menu_texts_.size()))
    {
        menu_texts_[selected_index_].setFillColor(sf::Color::White);
        selected_index_++;
        menu_texts_[selected_index_].setFillColor(sf::Color::Yellow);
    }
}