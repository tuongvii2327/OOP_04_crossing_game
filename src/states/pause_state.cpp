#include "states/pause_state.h"
#include "states/home_state.h"
#include "states/save_state.h"
#include "states/setting_state.h"
#include "utils/audio_system.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"

PauseState::PauseState(SharedContext* context) : State(context), selected_index_(0) {}

// Load font and initialize UI
void PauseState::init()
{
    Audio().resumeMusic();
    const sf::Font& font = Resource().getFont("assets/fonts/VT323-Regular.ttf");
    title_text_ = std::make_unique<sf::Text>(font);

    setupUI(font);
}

// Setup pause menu UI with overlay and options
void PauseState::setupUI(const sf::Font& font)
{
    background_overlay_.setSize({static_cast<float>(Constants::WINDOW_WIDTH),
                                 static_cast<float>(Constants::WINDOW_HEIGHT)});
    background_overlay_.setFillColor(sf::Color(0, 0, 0, 150));

    *title_text_ = Resource().createText("PAUSED", {120});

    sf::FloatRect title_bounds = title_text_->getLocalBounds();
    title_text_->setPosition({Constants::WINDOW_WIDTH / 2.0f, 100.0f + title_bounds.size.y / 2.0f});

    std::vector<std::string> options = {"RESUME", "SAVE GAME", "SETTINGS", "HOME"};
    float initial_y = 400.0f;
    float spacing = 100.0f;

    for (size_t i = 0; i < options.size(); ++i)
    {
        std::unique_ptr<sf::Text> option_text =
            std::make_unique<sf::Text>(Resource().createText(options[i], {70}, true));

        if (static_cast<int>(i) == selected_index_)
        {
            Resource().setTextButtonHovered(*option_text, true);
        }

        sf::FloatRect text_bounds = option_text->getLocalBounds();
        float x_position = Constants::WINDOW_WIDTH / 2.0f;
        float y_position = initial_y + (static_cast<float>(i) * spacing) + text_bounds.size.y / 2.0f;

        option_text->setPosition({x_position, y_position});
        menu_texts_.push_back(std::move(option_text));
    }
}

// Handle pause menu input
void PauseState::handleEvent(const sf::Event& event)
{
    if (const auto* key_pressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (key_pressed->code == sf::Keyboard::Key::Up || key_pressed->code == sf::Keyboard::Key::W)
        {
            moveUp();
            Audio().playSFX(AudioSystem::SFX::Hover);
        }
        else if (key_pressed->code == sf::Keyboard::Key::Down ||
                 key_pressed->code == sf::Keyboard::Key::S)
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
    if (const auto* mouse_moved = event.getIf<sf::Event::MouseMoved>())
    {
        sf::Vector2f mouse_pos = context_->window->mapPixelToCoords(mouse_moved->position);

        for (size_t i = 0; i < menu_texts_.size(); ++i)
        {
            if (menu_texts_[i]->getGlobalBounds().contains(mouse_pos))
            {
                // change color to yellow for selected option and white for others
                if (selected_index_ != static_cast<int>(i))
                {
                    Resource().setTextButtonHovered(*menu_texts_[selected_index_], false);
                    selected_index_ = static_cast<int>(i);
                    Resource().setTextButtonHovered(*menu_texts_[selected_index_], true);
                    Audio().playSFX(AudioSystem::SFX::Hover);
                }
                break;
            }
        }
    }

    // Handle left mouse button click (MouseButtonPressed)
    if (const auto* mouse_button_pressed = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mouse_button_pressed->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mouse_pos = context_->window->mapPixelToCoords(mouse_button_pressed->position);

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
void PauseState::render(sf::RenderWindow& window)
{
    window.draw(background_overlay_);
    if (title_text_)
    {
        Resource().drawTextWithShadow(window, *title_text_);
    }
    for (const auto& menu_text : menu_texts_)
    {
        if (menu_text)
        {
            Resource().drawTextWithShadow(window, *menu_text);
        }
    }
}

// Move selection up
void PauseState::moveUp()
{
    Resource().setTextButtonHovered(*menu_texts_[selected_index_], false);
    selected_index_--;
    if (selected_index_ < 0)
        selected_index_ = static_cast<int>(menu_texts_.size()) - 1;
    Resource().setTextButtonHovered(*menu_texts_[selected_index_], true);
}

// Move selection down
void PauseState::moveDown()
{
    Resource().setTextButtonHovered(*menu_texts_[selected_index_], false);
    selected_index_++;
    if (selected_index_ >= static_cast<int>(menu_texts_.size()))
        selected_index_ = 0;
    Resource().setTextButtonHovered(*menu_texts_[selected_index_], true);
}
