#include "states/home_state.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"
#include <iostream>

#include "states/load_state.h"
#include "states/playing_state.h"
#include "states/ranking_state.h"
#include "states/setting_state.h"
#include "utils/audio_system.h"
#include "utils/shared_context.h"

HomeState::HomeState(SharedContext *context)
    : State(context), selected_index_(0)
{
}

// Apply text formatting and center positioning
void HomeState::setupMenuOption(sf::Text &text, const std::string &str, float y_position)
{
    text.setString(str);
    text.setCharacterSize(40);
    text.setFillColor(sf::Color::White);

    text.setOutlineColor(sf::Color::Black);
    text.setOutlineThickness(3.0f);

    sf::FloatRect text_rect = text.getLocalBounds();
    text.setOrigin({text_rect.size.x / 2.0f, text_rect.size.y / 2.0f});
    text.setPosition({Constants::WINDOW_WIDTH / 2.0f, y_position});
}

// Load font and initialize menu UI
void HomeState::init()
{
    const sf::Font &font = Resource().getFont("assets/fonts/VT323-Regular.ttf");

    title_text_ = std::make_unique<sf::Text>(font);
    title_text_->setString(Constants::GAME_TITLE);
    title_text_->setCharacterSize(80);
    title_text_->setFillColor(sf::Color::Green);
    sf::FloatRect title_rect = title_text_->getLocalBounds();
    title_text_->setOrigin({title_rect.size.x / 2.0f, title_rect.size.y / 2.0f});
    title_text_->setPosition({Constants::WINDOW_WIDTH / 2.0f, 200.0f});

    std::vector<std::pair<std::string, float>> menu_data = {
        {"New Game", 450.0f}, {"Load File", 550.0f}, {"Settings", 650.0f}, {"Ranking", 750.0f}, {"Quit", 850.0f}};

    menu_options_.clear();
    for (const auto &data : menu_data)
    {
        auto text_ptr = std::make_unique<sf::Text>(font);
        setupMenuOption(*text_ptr, data.first, data.second);
        menu_options_.push_back(std::move(text_ptr));
    }

    menu_options_[selected_index_]->setFillColor(sf::Color::Yellow);
}

// Handle keyboard navigation and mouse interaction
void HomeState::handleEvent(const sf::Event &event)
{
    if (const auto *key_pressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (key_pressed->code == sf::Keyboard::Key::Up || key_pressed->code == sf::Keyboard::Key::W)
        {
            Audio().playSFX(AudioSystem::SFX::Hover);
            menu_options_[selected_index_]->setFillColor(sf::Color::White);
            selected_index_ = (selected_index_ - 1 + menu_options_.size()) % menu_options_.size();
            menu_options_[selected_index_]->setFillColor(sf::Color::Yellow);
        }
        else if (key_pressed->code == sf::Keyboard::Key::Down || key_pressed->code == sf::Keyboard::Key::S)
        {
            Audio().playSFX(AudioSystem::SFX::Hover);
            menu_options_[selected_index_]->setFillColor(sf::Color::White);
            selected_index_ = (selected_index_ + 1) % menu_options_.size();
            menu_options_[selected_index_]->setFillColor(sf::Color::Yellow);
        }
        else if (key_pressed->code == sf::Keyboard::Key::Enter)
        {
            Audio().playSFX(AudioSystem::SFX::Click);
            executeChoice(selected_index_);
        }
    }

    if (const auto *mouse_moved = event.getIf<sf::Event::MouseMoved>())
    {
        sf::Vector2f mouse_pos(static_cast<float>(mouse_moved->position.x), static_cast<float>(mouse_moved->position.y));

        for (size_t i = 0; i < menu_options_.size(); ++i)
        {
            if (menu_options_[i]->getGlobalBounds().contains(mouse_pos))
            {
                // Chỉ đổi màu và phát tiếng khi chuột di chuyển sang một menu khác
                if (selected_index_ != static_cast<int>(i))
                {
                    menu_options_[selected_index_]->setFillColor(sf::Color::White); // Trả chữ cũ về màu trắng
                    selected_index_ = static_cast<int>(i);
                    menu_options_[selected_index_]->setFillColor(sf::Color::Yellow); // Đổi chữ mới thành màu vàng
                    Audio().playSFX(AudioSystem::SFX::Hover);                        // Phát âm thanh Hover
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

            for (size_t i = 0; i < menu_options_.size(); ++i)
            {
                if (menu_options_[i]->getGlobalBounds().contains(mouse_pos))
                {
                    selected_index_ = static_cast<int>(i);
                    Audio().playSFX(AudioSystem::SFX::Click);
                    executeChoice(selected_index_);
                    break;
                }
            }
        }
    }
}

// Update logic (static menu, no-op)
void HomeState::update(float dt) {}

// Draw title and menu options
void HomeState::render(sf::RenderWindow &window)
{
    if (title_text_)
    {
        window.draw(*title_text_);
    }

    for (const auto &option : menu_options_)
    {
        window.draw(*option);
    }
}

// Execute menu selection
void HomeState::executeChoice(int index)
{
    switch (index)
    {
    case 0:
        std::cout << "Transition to: Playing\n";
        context_->state_machine->push(std::make_unique<PlayingState>(context_));
        break;
    case 1:
        std::cout << "Transition to: Load File\n";
        context_->state_machine->push(std::make_unique<LoadState>(context_));
        break;
    case 2:
        std::cout << "Transition to: Settings\n";
        context_->state_machine->push(std::make_unique<SettingState>(context_));
        break;
    case 3:
        std::cout << "Transition to: Ranking\n";
        context_->state_machine->push(std::make_unique<RankingState>(context_));
        break;
    case 4:
        std::cout << "Exit game!\n";
        context_->window->close();
        break;
    }
}
