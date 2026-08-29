#include "states/setting_state.h"
#include "utils/audio_system.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"
#include "utils/shared_context.h"
#include <SFML/Graphics/RenderWindow.hpp>

SettingState::SettingState(SharedContext* context) : State(context), selected_index_(0) {}

void SettingState::init()
{
    const sf::Texture& bg_tex = Resource().getTexture("assets/textures/ui/setting.png");
    background_sprite_ = std::make_unique<sf::Sprite>(bg_tex);
    background_sprite_->setTexture(bg_tex, true);
    background_sprite_->setTextureRect(
        sf::IntRect({0, 0}, sf::Vector2i(static_cast<int>(bg_tex.getSize().x),
                                         static_cast<int>(bg_tex.getSize().y))));
    background_sprite_->setPosition({0.f, 0.f});

    float scale_x =
        static_cast<float>(Constants::WINDOW_WIDTH) / static_cast<float>(bg_tex.getSize().x);
    float scale_y =
        static_cast<float>(Constants::WINDOW_HEIGHT) / static_cast<float>(bg_tex.getSize().y);
    background_sprite_->setScale({scale_x, scale_y});

    const sf::Font& font = Resource().getFont("assets/fonts/VT323-Regular.ttf");

    title_text_ = std::make_unique<sf::Text>(font);

    setupUI();
}

std::vector<std::string> SettingState::buildDisplayStrings() const
{
    return {"MUSIC VOLUME: < " + std::to_string(context_->config->music_volume * 10) + "% >",
            "SFX VOLUME: < " + std::to_string(context_->config->sfx_volume * 10) + "% >",
            "AUTO SCROLL: < " + std::string(context_->config->auto_scroll ? "ON" : "OFF") + " >",
            "BACK TO MENU"}; // phai luon de back o cuoi
}

void SettingState::setupUI()
{
    *title_text_ = Resource().createText("SETTINGS", {72, sf::Color::Magenta});
    sf::FloatRect title_bounds = title_text_->getLocalBounds();
    title_text_->setPosition({Constants::WINDOW_WIDTH / 2.0f, 150.0f + title_bounds.size.y / 2.0f});

    setting_texts_.clear();
    size_t item_count = buildDisplayStrings().size();
    for (int i = 0; i < item_count; ++i)
    {
        setting_texts_.push_back(std::make_unique<sf::Text>(Resource().createText("", {70}, true)));
    }

    updateUI();
}

void SettingState::updateUI()
{
    int mus = context_->config->music_volume;
    int sfx = context_->config->sfx_volume;
    bool auto_scr = context_->config->auto_scroll;

    std::vector<std::string> display_strings = buildDisplayStrings();

    float initial_y = 350.0f;
    float spacing = 100.0f;

    for (size_t i = 0; i < setting_texts_.size(); ++i)
    {
        setting_texts_[i]->setString(display_strings[i]);

        sf::FloatRect text_bounds = setting_texts_[i]->getLocalBounds();
        setting_texts_[i]->setOrigin({text_bounds.size.x / 2.0f, text_bounds.size.y / 2.0f});
        Resource().setTextButtonHovered(*setting_texts_[i], static_cast<int>(i) == selected_index_);

        float x_position = Constants::WINDOW_WIDTH / 2.0f;
        float y_position = initial_y + (static_cast<float>(i) * spacing) + text_bounds.size.y / 2.0f;

        setting_texts_[i]->setPosition({x_position, y_position});
    }
}

void SettingState::handleEvent(const sf::Event& event)
{
    if (const auto* key_pressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (key_pressed->code == sf::Keyboard::Key::Up || key_pressed->code == sf::Keyboard::Key::W)
        {
            moveUp();
        }
        else if (key_pressed->code == sf::Keyboard::Key::Down ||
                 key_pressed->code == sf::Keyboard::Key::S)
        {
            moveDown();
        }
        else if (key_pressed->code == sf::Keyboard::Key::Left ||
                 key_pressed->code == sf::Keyboard::Key::A)
        {
            decreaseSetting();
        }
        else if (key_pressed->code == sf::Keyboard::Key::Right ||
                 key_pressed->code == sf::Keyboard::Key::D)
        {
            increaseSetting();
        }
        else if (key_pressed->code == sf::Keyboard::Key::Enter)
        {
            if (selected_index_ == static_cast<int>(setting_texts_.size()) - 1)
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
            Audio().playSFX(AudioSystem::SFX::Back);
            context_->state_machine->pop();
        }
    }

    if (const auto* mouse_moved = event.getIf<sf::Event::MouseMoved>())
    {
        sf::Vector2f mouse_pos = context_->window->mapPixelToCoords(mouse_moved->position);

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

    if (const auto* mouse_button_pressed = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mouse_button_pressed->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mouse_pos = context_->window->mapPixelToCoords(mouse_button_pressed->position);
    
            for (size_t i = 0; i < setting_texts_.size(); ++i)
            {
                if (setting_texts_[i]->getGlobalBounds().contains(mouse_pos))
                {
                    selected_index_ = static_cast<int>(i);
    
                    if (selected_index_ == static_cast<int>(setting_texts_.size()) - 1)
                    {
                        Audio().playSFX(AudioSystem::SFX::Back);
                        context_->state_machine->pop(); // Click BACK TO MENU
                    }
                    else
                    {
                        // ─── TÌM TỌA ĐỘ PIXEL THỰC TẾ CỦA KÝ TỰ '<' VÀ '>' BẰNG SFML ───
                        std::string str = setting_texts_[i]->getString().toAnsiString();
                        size_t left_idx = str.find('<');
                        size_t right_idx = str.find('>');
    
                        if (left_idx != std::string::npos && right_idx != std::string::npos)
                        {
                            float left_arrow_x = setting_texts_[i]->findCharacterPos(left_idx).x;
                            float right_arrow_x = setting_texts_[i]->findCharacterPos(right_idx).x;
    
                            // Điểm chia ranh giới giữa mũi tên trái và mũi tên phải
                            float mid_arrow_x = (left_arrow_x + right_arrow_x) / 2.0f;
    
                            if (mouse_pos.x < mid_arrow_x)
                            {
                                decreaseSetting(); // Click gần mũi tên trái '<' -> Giảm Âm lượng
                            }
                            else
                            {
                                increaseSetting(); // Click gần mũi tên phải '>' -> Tăng Âm lượng
                            }
                        }
                        else
                        {
                            // Fallback cho Auto Scroll (ON / OFF không có mũi tên)
                            increaseSetting();
                        }
                    }
                    break;
                }
            }
        }
    }
}

void SettingState::update(float delta_time) {}

void SettingState::render(sf::RenderWindow& window)
{
    window.clear(sf::Color(25, 25, 25));

    if (background_sprite_)
    {
        window.draw(*background_sprite_);
    }

    // if (title_text_)
    // {
    //     window.draw(*title_text_);
    // }

    for (const auto& setting_text : setting_texts_)
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
        Audio().setMusicVolume(context_->config->music_volume);
        break;
    case 1:
        context_->config->sfx_volume = (context_->config->sfx_volume + 1) % 11;
        Audio().setSFXVolume(context_->config->sfx_volume);
        break;
    case 2:
        context_->config->auto_scroll = !context_->config->auto_scroll;
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
        Audio().setMusicVolume(context_->config->music_volume);
        break;
    case 1:
        context_->config->sfx_volume = (context_->config->sfx_volume - 1 + 11) % 11;
        Audio().setSFXVolume(context_->config->sfx_volume);
        break;
    case 2:
        context_->config->auto_scroll = !context_->config->auto_scroll;
        break;
    }
    updateUI();
}
