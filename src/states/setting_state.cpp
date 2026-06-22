#include "states/setting_state.h"
#include "utils/constants.h"
#include "utils/resource_manager.h" // Nhúng kho quản lý tài nguyên
#include <iostream>

SettingState::SettingState(StateMachine &state_machine)
    : state_machine_(state_machine),
      selected_index_(0),
      music_volume_(5),
      sfx_volume_(7),
      is_hard_mode_(false)
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
    for (int i = 0; i < 4; ++i)
    {
        setting_texts_.push_back(std::make_unique<sf::Text>(font));
    }

    updateUI();
}

void SettingState::updateUI()
{
    std::vector<std::string> display_strings = {
        "MUSIC VOLUME: < " + std::to_string(music_volume_ * 10) + "% >",
        "SFX VOLUME: < " + std::to_string(sfx_volume_ * 10) + "% >",
        "DIFFICULTY: < " + std::string(is_hard_mode_ ? "HARD" : "NORMAL") + " >",
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
            if (selected_index_ == 3)
            {
                state_machine_.removeState();
            }
            else
            {
                increaseSetting();
            }
        }
        else if (key_pressed->code == sf::Keyboard::Key::Escape)
        {
            state_machine_.removeState();
        }
    }
}

void SettingState::update(float delta_time)
{
}

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
    if (selected_index_ - 1 >= 0)
    {
        selected_index_--;
        updateUI();
    }
}

void SettingState::moveDown()
{
    if (selected_index_ + 1 < static_cast<int>(setting_texts_.size()))
    {
        selected_index_++;
        updateUI();
    }
}

void SettingState::increaseSetting()
{
    switch (selected_index_)
    {
    case 0:
        if (music_volume_ < 10)
            music_volume_++;
        break;
    case 1:
        if (sfx_volume_ < 10)
            sfx_volume_++;
        break;
    case 2:
        is_hard_mode_ = !is_hard_mode_;
        break;
    default:
        return;
    }
    updateUI();
}

void SettingState::decreaseSetting()
{
    switch (selected_index_)
    {
    case 0:
        if (music_volume_ > 0)
            music_volume_--;
        break;
    case 1:
        if (sfx_volume_ > 0)
            sfx_volume_--;
        break;
    case 2:
        is_hard_mode_ = !is_hard_mode_;
        break;
    default:
        return;
    }
    updateUI();
}