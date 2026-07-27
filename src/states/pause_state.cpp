#include "states/pause_state.h"
#include "states/home_state.h"
#include "states/save_state.h"
#include "states/setting_state.h"
#include "utils/audio_system.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"

PauseState::PauseState(SharedContext *context)
    : State(context), selected_index_(0)
{
}

// Load font and initialize UI
void PauseState::init()
{
    Audio().resumeMusic();
    const sf::Font &font = Resource().getFont("assets/fonts/VT323-Regular.ttf");
    title_text_ = std::make_unique<sf::Text>(font);

    setupUI(font);
}

// Setup pause menu UI with overlay and options
void PauseState::setupUI(const sf::Font &font)
{
    background_overlay_.setSize({static_cast<float>(Constants::WINDOW_WIDTH),
                                 static_cast<float>(Constants::WINDOW_HEIGHT)});
    background_overlay_.setFillColor(sf::Color(0, 0, 0, 150));

    title_text_->setString("PAUSED");
    title_text_->setCharacterSize(90);
    title_text_->setFillColor(sf::Color::White);

    sf::FloatRect title_bounds = title_text_->getLocalBounds();
    float title_x = (Constants::WINDOW_WIDTH - title_bounds.size.x) / 2.0f;
    title_text_->setPosition({title_x, 200.0f});

    std::vector<std::string> options = {"RESUME", "SAVE GAME", "SETTINGS", "HOME"};
    float initial_y = 450.0f;
    float spacing = 80.0f;

    for (size_t i = 0; i < options.size(); ++i)
    {
        std::unique_ptr<sf::Text> option_text = std::make_unique<sf::Text>(font);
        option_text->setString(options[i]);
        option_text->setCharacterSize(50);

        if (static_cast<int>(i) == selected_index_)
        {
            option_text->setFillColor(sf::Color::Yellow);
        }
        else
        {
            option_text->setFillColor(sf::Color::White);
        }

        sf::FloatRect text_bounds = option_text->getLocalBounds();
        float x_position = (Constants::WINDOW_WIDTH - text_bounds.size.x) / 2.0f;
        float y_position = initial_y + (static_cast<float>(i) * spacing);

        option_text->setPosition({x_position, y_position});
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
            Audio().playSFX(AudioSystem::SFX::Hover);
        }
        else if (key_pressed->code == sf::Keyboard::Key::Down || key_pressed->code == sf::Keyboard::Key::S)
        {
            moveDown();
            Audio().playSFX(AudioSystem::SFX::Hover);
        }
        else if (key_pressed->code == sf::Keyboard::Key::Enter)
        {
            Audio().playSFX(AudioSystem::SFX::Click);
            if (selected_index_ == 0)
            {
                context_->state_machine->pop(); // Resume
            }
            else if (selected_index_ == 1)
            {
                context_->state_machine->push(std::make_unique<SaveState>(context_));
            }
            else if (selected_index_ == 2)
            {
                context_->state_machine->push(std::make_unique<SettingState>(context_));
            }
            else if (selected_index_ == 3)
            {
                context_->state_machine->resetTo(std::make_unique<HomeState>(context_));
            }
        }
        else if (key_pressed->code == sf::Keyboard::Key::Escape)
        {
            Audio().playSFX(AudioSystem::SFX::Back);
            context_->state_machine->pop(); // Resume
        }
    }
    // MouseMoved
    if (const auto *mouse_moved = event.getIf<sf::Event::MouseMoved>())
    {
        sf::Vector2f mouse_pos(static_cast<float>(mouse_moved->position.x), static_cast<float>(mouse_moved->position.y));

        for (size_t i = 0; i < menu_texts_.size(); ++i)
        {
            if (menu_texts_[i]->getGlobalBounds().contains(mouse_pos))
            {
                // change color to yellow for selected option and white for others
                if (selected_index_ != static_cast<int>(i))
                {
                    menu_texts_[selected_index_]->setFillColor(sf::Color::White);
                    selected_index_ = static_cast<int>(i);
                    menu_texts_[selected_index_]->setFillColor(
                        sf::Color::Yellow);
                    Audio().playSFX(AudioSystem::SFX::Hover);
                }
                break;
            }
        }
    }

    // Handle left mouse button click (MouseButtonPressed)
    if (const auto *mouse_button_pressed = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mouse_button_pressed->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mouse_pos(static_cast<float>(mouse_button_pressed->position.x), static_cast<float>(mouse_button_pressed->position.y));

            for (size_t i = 0; i < menu_texts_.size(); ++i)
            {
                if (menu_texts_[i]->getGlobalBounds().contains(mouse_pos))
                {
                    selected_index_ = static_cast<int>(i);
                    if (selected_index_ == 0)
                    {
                        context_->state_machine->pop(); // Resume
                    }
                    else if (selected_index_ == 1)
                    {
                        context_->state_machine->push(std::make_unique<SaveState>(context_));
                    }
                    else if (selected_index_ == 2)
                    {
                        context_->state_machine->push(std::make_unique<SettingState>(context_));
                    }
                    else if (selected_index_ == 3)
                    {
                        context_->state_machine->resetTo(std::make_unique<HomeState>(context_));
                    }
                    break;
                }
            }
        }
    }
}

// Update pause state (static)
void PauseState::update(float delta_time) {}

// Render pause menu overlay and options
void PauseState::render(sf::RenderWindow &window)
{
    window.draw(background_overlay_);
    if (title_text_)
    {
        window.draw(*title_text_);
    }
    for (const auto &menu_text : menu_texts_)
    {
        if (menu_text)
        {
            window.draw(*menu_text);
        }
    }
}

// Move selection up
void PauseState::moveUp()
{
    menu_texts_[selected_index_]->setFillColor(sf::Color::White);
    selected_index_--;
    if (selected_index_ < 0)
        selected_index_ = static_cast<int>(menu_texts_.size()) - 1;
    menu_texts_[selected_index_]->setFillColor(sf::Color::Yellow);
}

// Move selection down
void PauseState::moveDown()
{
    menu_texts_[selected_index_]->setFillColor(sf::Color::White);
    selected_index_++;
    if (selected_index_ >= static_cast<int>(menu_texts_.size()))
        selected_index_ = 0;
    menu_texts_[selected_index_]->setFillColor(sf::Color::Yellow);
}
