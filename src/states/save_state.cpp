// src/states/save_state.cpp
#include "states/save_state.h"
#include "utils/constants.h"
#include <iostream>

SaveState::SaveState(StateMachine &state_machine)
    : state_machine_(state_machine),
      selected_index_(0),
      is_saved_(false),
      save_timer_(0.0f)
{
}

// Load font and initialize save screen
void SaveState::init()
{
    const sf::Font &font = Resource().getFont("assets/fonts/VT323-Regular.ttf");

    title_text_ = std::make_unique<sf::Text>(font);

    setupUI(font);
}

// Setup save menu UI with save slots
void SaveState::setupUI(const sf::Font &font)
{
    title_text_->setString("SAVE GAME");
    title_text_->setCharacterSize(72);
    title_text_->setFillColor(sf::Color::Green);

    sf::FloatRect title_bounds = title_text_->getLocalBounds();
    float title_x = (Constants::WINDOW_WIDTH - title_bounds.size.x) / 2.0f;
    title_text_->setPosition({title_x, 150.0f});

    std::vector<std::string> save_options = {
        "SAVE SLOT 1",
        "SAVE SLOT 2",
        "SAVE SLOT 3",
        "BACK"};

    float initial_y = 350.0f;
    float spacing = 80.0f;

    for (size_t i = 0; i < save_options.size(); ++i)
    {
        auto slot_text = std::make_unique<sf::Text>(font);
        slot_text->setString(save_options[i]);
        slot_text->setCharacterSize(48);

        if (static_cast<int>(i) == selected_index_)
        {
            slot_text->setFillColor(sf::Color::Yellow);
        }
        else
        {
            slot_text->setFillColor(sf::Color::White);
        }

        sf::FloatRect text_bounds = slot_text->getLocalBounds();
        float x_position = (Constants::WINDOW_WIDTH - text_bounds.size.x) / 2.0f;
        float y_position = initial_y + (static_cast<float>(i) * spacing);

        slot_text->setPosition({x_position, y_position});
        save_slot_texts_.push_back(std::move(slot_text));
    }
}

// Handle slot selection and navigation
void SaveState::handleEvent(const sf::Event &event)
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
            if (selected_index_ == static_cast<int>(save_slot_texts_.size()) - 1)
            {
                state_machine_.removeState();
            }
            else
            {
                saveGameData(selected_index_ + 1);
            }
        }
        else if (key_pressed->code == sf::Keyboard::Key::Escape)
        {
            state_machine_.removeState();
        }
    }
}

// Update save state (static)
void SaveState::update(float delta_time)
{
    if (is_saved_)
    {
        save_timer_ += delta_time;
        if (save_timer_ >= 1.0f) // wait for 1 second before returning to PauseState
        {
            state_machine_.removeState();
        }
    }
}

// Render save menu
void SaveState::render(sf::RenderWindow &window)
{
    window.clear(sf::Color(25, 25, 25));

    window.draw(*title_text_);

    for (const auto &slot_text : save_slot_texts_)
    {
        window.draw(*slot_text);
    }
}

// Move selection up
void SaveState::moveUp()
{
    if (selected_index_ - 1 >= 0)
    {
        save_slot_texts_[selected_index_]->setFillColor(sf::Color::White);
        selected_index_--;
        save_slot_texts_[selected_index_]->setFillColor(sf::Color::Yellow);
    }
}

// Move selection down
void SaveState::moveDown()
{
    if (selected_index_ + 1 < static_cast<int>(save_slot_texts_.size()))
    {
        save_slot_texts_[selected_index_]->setFillColor(sf::Color::White);
        selected_index_++;
        save_slot_texts_[selected_index_]->setFillColor(sf::Color::Yellow);
    }
}

// Save game data to save file
void SaveState::saveGameData(int slot_index)
{
    // std::cout << "Successfully saved game to Slot " << slot_index << "!" << std::endl;

    save_slot_texts_[slot_index - 1]->setString("SAVE SLOT " + std::to_string(slot_index) + " - SAVED!");

    sf::FloatRect text_bounds = save_slot_texts_[slot_index - 1]->getLocalBounds();
    float x_position = (Constants::WINDOW_WIDTH - text_bounds.size.x) / 2.0f;
    float y_position = save_slot_texts_[slot_index - 1]->getPosition().y;
    save_slot_texts_[slot_index - 1]->setPosition({x_position, y_position});
    is_saved_ = true;
    save_timer_ = 0.0f;
}