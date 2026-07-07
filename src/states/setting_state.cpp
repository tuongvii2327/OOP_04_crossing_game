#include "states/setting_state.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"
#include "utils/shared_context.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <iostream>

SettingState::SettingState(SharedContext *context)
    : State(context),
      selected_index_(0)
{
}

void SettingState::init()
{
    const sf::Font &font = Resource().getFont("assets/fonts/VT323-Regular.ttf");

    title_text_ = std::make_unique<sf::Text>(font);

    setupUI(font);
}

void SettingState::setupUI(const sf::Font &font)
{
    title_text_->setString("SETTINGS");
    title_text_->setCharacterSize(72);
    title_text_->setFillColor(sf::Color::Magenta);

    sf::FloatRect title_bounds = title_text_->getLocalBounds();
    float title_x = (Constants::WINDOW_WIDTH - title_bounds.size.x) / 2.0f;
    title_text_->setPosition({title_x, 150.0f});

    setting_texts_.clear();
    for (int i = 0; i < 3; ++i)
    {
        setting_texts_.push_back(std::make_unique<sf::Text>(font));
    }

    updateUI();
}

void SettingState::updateUI()
{
    int mus = context_->config->music_volume;
    int sfx = context_->config->sfx_volume;

    std::vector<std::string> display_strings = {
        "MUSIC VOLUME: < " + std::to_string(mus * 10) + "% >",
        "SFX VOLUME: < " + std::to_string(sfx * 10) + "% >",
        "BACK TO MENU"};

    float initial_y = 350.0f;
    float spacing = 80.0f;

    for (size_t i = 0; i < setting_texts_.size(); ++i)
    {
        setting_texts_[i]->setString(display_strings[i]);

        if (static_cast<int>(i) == selected_index_)
        {
            setting_texts_[i]->setFillColor(sf::Color::Yellow);
        }
        else
        {
            setting_texts_[i]->setFillColor(sf::Color::White);
        }

        sf::FloatRect text_bounds = setting_texts_[i]->getLocalBounds();
        float x_position = (Constants::WINDOW_WIDTH - text_bounds.size.x) / 2.0f;
        float y_position = initial_y + (static_cast<float>(i) * spacing);

        setting_texts_[i]->setPosition({x_position, y_position});
    }
}

void SettingState::handleEvent(const sf::Event &event)
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
        else if (key_pressed->code == sf::Keyboard::Key::Left || key_pressed->code == sf::Keyboard::Key::A)
        {
            decreaseSetting();
        }
        else if (key_pressed->code == sf::Keyboard::Key::Right || key_pressed->code == sf::Keyboard::Key::D)
        {
            increaseSetting();
        }
        else if (key_pressed->code == sf::Keyboard::Key::Enter)
        {
            if (selected_index_ == 2)
            {
                context_->state_machine->pop();
            }
            else
            {
                increaseSetting();
            }
        }
        else if (key_pressed->code == sf::Keyboard::Key::Escape)
        {
            context_->state_machine->pop();
        }
    }

    if (const auto *mouse_moved = event.getIf<sf::Event::MouseMoved>())
    {
        sf::Vector2f mouse_pos(static_cast<float>(mouse_moved->position.x), static_cast<float>(mouse_moved->position.y));

        for (size_t i = 0; i < setting_texts_.size(); ++i)
        {
            if (setting_texts_[i]->getGlobalBounds().contains(mouse_pos))
            {
                if (selected_index_ != static_cast<int>(i))
                {
                    selected_index_ = static_cast<int>(i);
                    updateUI();
                }
                break;
            }
        }
    }

    if (const auto *mouse_button_pressed = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mouse_button_pressed->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mouse_pos(static_cast<float>(mouse_button_pressed->position.x), static_cast<float>(mouse_button_pressed->position.y));

            for (size_t i = 0; i < setting_texts_.size(); ++i)
            {
                if (setting_texts_[i]->getGlobalBounds().contains(mouse_pos))
                {
                    if (selected_index_ == 2)
                    {
                        context_->state_machine->pop();
                    }
                    else
                    {
                        increaseSetting();
                    }
                    break;
                }
            }
        }
    }
}

void SettingState::update(float delta_time) {}

void SettingState::render(sf::RenderWindow &window)
{
    window.clear(sf::Color(25, 25, 25));

    if (title_text_)
    {
        window.draw(*title_text_);
    }

    for (const auto &setting_text : setting_texts_)
    {
        if (setting_text)
        {
            window.draw(*setting_text);
        }
    }
}

void SettingState::moveUp()
{
    selected_index_--;
    if (selected_index_ < 0)
    {
        selected_index_ = static_cast<int>(setting_texts_.size()) - 1;
    }
    updateUI();
}

void SettingState::moveDown()
{
    selected_index_++;
    if (selected_index_ >= static_cast<int>(setting_texts_.size()))
    {
        selected_index_ = 0;
    }
    updateUI();
}

void SettingState::increaseSetting()
{
    switch (selected_index_)
    {
    case 0:
        context_->config->music_volume = (context_->config->music_volume + 1) % 11;
        break;
    case 1:
        context_->config->sfx_volume = (context_->config->sfx_volume + 1) % 11;
        break;
    }
    updateUI();
}

void SettingState::decreaseSetting()
{
    switch (selected_index_)
    {
    case 0:
        context_->config->music_volume = (context_->config->music_volume - 1 + 11) % 11;
        break;
    case 1:
        context_->config->sfx_volume = (context_->config->sfx_volume - 1 + 11) % 11;
        break;
    }
    updateUI();
}
