#include "states/home_state.h"
#include "states/loading_state.h"
#include "states/shop_state.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"
#include <iostream>

#include "states/load_state.h"
#include "states/playing_state.h"
#include "states/ranking_state.h"
#include "states/setting_state.h"
#include "utils/audio_system.h"
#include "utils/shared_context.h"
#include "states/help_state.h"
#include "states/about_state.h"
#include "states/credit_state.h"
HomeState::HomeState(SharedContext* context) : State(context), selected_index_(0) {}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void HomeState::setupMenuOption(sf::Text& text, const std::string& str, float y_position)
{
    text = Resource().createText(str, {85}, true);
    text.setPosition({Constants::WINDOW_WIDTH / 2.0f, y_position});
}

// icon_buttons_ layout:
//   [0] SFX    (choice 6)   – Top-Right
//   [1] Music  (choice 7)   – Top-Right
//   [2] Settings (choice 3) – Top-Right
//   [3] Exit   (choice 5)   – Bottom-Left
//   [4] Shop   (choice 1)   – Bottom-Right
//   [5] Ranking(choice 4)   – Bottom-Right
void HomeState::updateSoundIconTextures()
{
    constexpr float BASE_SIZE = 64.f;
    constexpr float HOVER_SIZE = 76.8f;

    // SFX icon (index 0): SpeakerOn / SpeakerMute
    bool sfx_on = Audio().getSFXVolume() > 0.f;
    const sf::Texture& sfx_tex = Resource().getTexture(
        sfx_on ? "assets/textures/icon/SpeakerOn.png" : "assets/textures/icon/SpeakerMute.png");
    icon_buttons_[0].sprite->setTexture(sfx_tex, true);
    icon_buttons_[0].sprite->setOrigin({sfx_tex.getSize().x / 2.f, sfx_tex.getSize().y / 2.f});
    float sfx_size = icon_buttons_[0].is_hovered ? HOVER_SIZE : BASE_SIZE;
    icon_buttons_[0].sprite->setScale({sfx_size / sfx_tex.getSize().x, sfx_size / sfx_tex.getSize().y});

    // Music icon (index 1): music_on / music_off
    bool music_on = Audio().getMusicVolume() > 0.f;
    const sf::Texture& music_tex = Resource().getTexture(
        music_on ? "assets/textures/icon/music_on.png" : "assets/textures/icon/music_off.png");
    icon_buttons_[1].sprite->setTexture(music_tex, true);
    icon_buttons_[1].sprite->setOrigin({music_tex.getSize().x / 2.f, music_tex.getSize().y / 2.f});
    float music_size = icon_buttons_[1].is_hovered ? HOVER_SIZE : BASE_SIZE;
    icon_buttons_[1].sprite->setScale({music_size / music_tex.getSize().x, music_size / music_tex.getSize().y});
}

void HomeState::initExitPopup()
{
    const float pw = 440.f, ph = 220.f;
    const float px = (Constants::WINDOW_WIDTH - pw) / 2.f;
    const float py = (Constants::WINDOW_HEIGHT - ph) / 2.f;

    // Full-screen dim overlay
    overlay_bg_ = std::make_unique<sf::RectangleShape>(sf::Vector2f(
        static_cast<float>(Constants::WINDOW_WIDTH), static_cast<float>(Constants::WINDOW_HEIGHT)));
    overlay_bg_->setFillColor(sf::Color(0, 0, 0, 150));
    overlay_bg_->setPosition({0.f, 0.f});

    // Popup box
    popup_bg_ = std::make_unique<sf::RectangleShape>(sf::Vector2f(pw, ph));
    popup_bg_->setFillColor(sf::Color(30, 30, 30, 230));
    popup_bg_->setOutlineColor(sf::Color::White);
    popup_bg_->setOutlineThickness(3.f);
    popup_bg_->setPosition({px, py});

    // Title text
    popup_title_ = std::make_unique<sf::Text>(Resource().createText("Exit game?", {60}));
    popup_title_->setPosition({px + pw / 2.f, py + 65.f});

    // Yes button
    popup_btn_yes_ = std::make_unique<sf::Text>(Resource().createText("Yes", {52}, true));
    popup_btn_yes_->setPosition({px + pw * 0.30f, py + 155.f});

    // No button
    popup_btn_no_ = std::make_unique<sf::Text>(Resource().createText("No", {52}, true));
    popup_btn_no_->setPosition({px + pw * 0.70f, py + 155.f});
    Resource().setTextButtonHovered(*popup_btn_no_, true);
}

void HomeState::showExitPopup()
{
    show_exit_popup_ = true;
    popup_selected_ = 1; // mặc định No
    popup_yes_hovered_ = false;
    popup_no_hovered_ = false;
    // No được chọn: No hover, Yes thường
    if (popup_btn_yes_)
        Resource().setTextButtonHovered(*popup_btn_yes_, false);
    if (popup_btn_no_)
        Resource().setTextButtonHovered(*popup_btn_no_, true);
}

// ---------------------------------------------------------------------------
// init
// ---------------------------------------------------------------------------

void HomeState::init()
{
    Audio().resumeMusic();

    // Background
    const sf::Texture& bg_tex = Resource().getTexture("assets/textures/ui/home.png");
    background_sprite_ = std::make_unique<sf::Sprite>(bg_tex);
    background_sprite_->setTexture(bg_tex, true);
    background_sprite_->setPosition({0.f, 0.f});
    background_sprite_->setScale(
        {static_cast<float>(Constants::WINDOW_WIDTH) / bg_tex.getSize().x,
         static_cast<float>(Constants::WINDOW_HEIGHT) / bg_tex.getSize().y});

    const sf::Font& font = Resource().getFont("assets/fonts/VT323-Regular.ttf");

    // Coin display (top-left)
    const sf::Texture& coin_tex = Resource().getTexture("assets/textures/icon/Coin.png");
    coin_sprite_ = std::make_unique<sf::Sprite>(coin_tex);
    coin_sprite_->setTexture(coin_tex, true);
    coin_sprite_->setOrigin({coin_tex.getSize().x / 2.f, coin_tex.getSize().y / 2.f});
    coin_sprite_->setPosition({40.f, 45.f});
    coin_sprite_->setScale({56.f / coin_tex.getSize().x, 56.f / coin_tex.getSize().y});

    coin_text_ = std::make_unique<sf::Text>(
        Resource().createText(std::to_string(context_->data->total_coins), {48, sf::Color::Yellow}));
    {
        sf::FloatRect r = coin_text_->getLocalBounds();
        coin_text_->setOrigin({0.f, r.size.y / 2.f});
        coin_text_->setPosition({70.f, 40.f});
    }

    // Central text menu
    std::vector<std::pair<std::string, float>> menu_data = {
        {"New Game", 320.f},
        {"Load File", 400.f},
        {"Help", 480.f},
        {"About", 560.f},
        {"Credits", 640.f}
    };
    menu_options_.clear();
    for (const auto& d : menu_data)
    {
        auto t = std::make_unique<sf::Text>(font);
        setupMenuOption(*t, d.first, d.second);
        menu_options_.push_back(std::move(t));
    }
    Resource().setTextButtonHovered(*menu_options_[selected_index_], true);

    // Corner icon buttons
    const float rx = static_cast<float>(Constants::WINDOW_WIDTH) - 55.f;
    const float by = static_cast<float>(Constants::WINDOW_HEIGHT) - 55.f;
    const float sp = 80.f;
    const float ty = 50.f;

    std::vector<IconConfig> cfgs = {
        // Top-Right: SFX, Music, Settings (right-to-left order)
        {"assets/textures/icon/SpeakerOn.png", 6, {rx - 2 * sp, ty}},
        {"assets/textures/icon/music_on.png", 7, {rx - sp, ty}},
        {"assets/textures/icon/Gear.png", 3, {rx, ty}},
        // Bottom-Left: Exit
        {"assets/textures/icon/Exit.png", 5, {55.f, by}},
        // Bottom-Right: Shop, Ranking
        {"assets/textures/icon/ShoppingCart.png", 1, {rx - sp, by}},
        {"assets/textures/icon/Trophy.png", 4, {rx, by}},
    };

    icon_buttons_.clear();
    for (const auto& c : cfgs)
    {
        IconButton btn;
        const sf::Texture& tex = Resource().getTexture(c.path);
        btn.sprite = std::make_unique<sf::Sprite>(tex);
        btn.sprite->setTexture(tex, true);
        btn.sprite->setOrigin({tex.getSize().x / 2.f, tex.getSize().y / 2.f});
        btn.sprite->setPosition(c.pos);
        btn.sprite->setScale({64.f / tex.getSize().x, 64.f / tex.getSize().y});
        btn.choice_index = c.choice;
        btn.is_hovered = false;
        icon_buttons_.push_back(std::move(btn));
    }

    updateSoundIconTextures();

    // Initialise exit popup (hidden by default)
    show_exit_popup_ = false;
    initExitPopup();
}

// ---------------------------------------------------------------------------
// handleEvent
// ---------------------------------------------------------------------------

void HomeState::handleEvent(const sf::Event& event)
{

    // ── Popup is visible: intercept all input ────────────────────────────────────────────
    if (show_exit_popup_)
    {
        if (const auto* kp = event.getIf<sf::Event::KeyPressed>())
        {
            if (kp->code == sf::Keyboard::Key::Escape)
            {
                show_exit_popup_ = false;
            }
            else if (kp->code == sf::Keyboard::Key::Left || kp->code == sf::Keyboard::Key::A ||
                     kp->code == sf::Keyboard::Key::Right || kp->code == sf::Keyboard::Key::D)
            {
                // Toggle giữa Yes (0) và No (1)
                popup_selected_ = 1 - popup_selected_;
                Audio().playSFX(AudioSystem::SFX::Hover);
                if (popup_selected_ == 0)
                {
                    // Yes đang chọn: Yes hover, No thường
                    if (popup_btn_yes_)
                        Resource().setTextButtonHovered(*popup_btn_yes_, true);
                    if (popup_btn_no_)
                        Resource().setTextButtonHovered(*popup_btn_no_, false);
                }
                else
                {
                    // No đang chọn: No hover, Yes thường
                    if (popup_btn_yes_)
                        Resource().setTextButtonHovered(*popup_btn_yes_, false);
                    if (popup_btn_no_)
                        Resource().setTextButtonHovered(*popup_btn_no_, true);
                }
            }
            else if (kp->code == sf::Keyboard::Key::Enter)
            {
                Audio().playSFX(AudioSystem::SFX::Click);
                if (popup_selected_ == 0)
                {
                    // std::cout << "Exit game!\n";
                    context_->window->close();
                }
                else
                {
                    show_exit_popup_ = false;
                }
            }
        }

        if (const auto* mm = event.getIf<sf::Event::MouseMoved>())
        {
            sf::Vector2f mp = context_->window->mapPixelToCoords(mm->position);
            // Hover Yes
            bool hy = popup_btn_yes_->getGlobalBounds().contains(mp);
            if (hy != popup_yes_hovered_)
            {
                popup_yes_hovered_ = hy;
                Resource().setTextButtonHovered(*popup_btn_yes_, hy);
                if (hy)
                    Audio().playSFX(AudioSystem::SFX::Hover);
            }
            // Hover No
            bool hn = popup_btn_no_->getGlobalBounds().contains(mp);
            if (hn != popup_no_hovered_)
            {
                popup_no_hovered_ = hn;
                Resource().setTextButtonHovered(*popup_btn_no_, hn);
                if (hn)
                    Audio().playSFX(AudioSystem::SFX::Hover);
            }
        }

        if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>())
        {
            if (mb->button == sf::Mouse::Button::Left)
            {
                sf::Vector2f mp = context_->window->mapPixelToCoords(mb->position);
                if (popup_btn_yes_->getGlobalBounds().contains(mp))
                {
                    Audio().playSFX(AudioSystem::SFX::Click);
                    // std::cout << "Exit game!\n";
                    context_->window->close();
                }
                else if (popup_btn_no_->getGlobalBounds().contains(mp))
                {
                    Audio().playSFX(AudioSystem::SFX::Click);
                    show_exit_popup_ = false;
                }
            }
        }
        return; // block all normal input while popup is open
    }

    // ── Normal input ─────────────────────────────────────────────────────────
    const int choice_map[] = {0, 2, 8, 9, 10};
    if (const auto* kp = event.getIf<sf::Event::KeyPressed>())
    {
        if (kp->code == sf::Keyboard::Key::Up || kp->code == sf::Keyboard::Key::W)
        {
            Audio().playSFX(AudioSystem::SFX::Hover);
            Resource().setTextButtonHovered(*menu_options_[selected_index_], false);
            selected_index_ = (selected_index_ - 1 + menu_options_.size()) % menu_options_.size();
            Resource().setTextButtonHovered(*menu_options_[selected_index_], true);
        }
        else if (kp->code == sf::Keyboard::Key::Down || kp->code == sf::Keyboard::Key::S)
        {
            Audio().playSFX(AudioSystem::SFX::Hover);
            Resource().setTextButtonHovered(*menu_options_[selected_index_], false);
            selected_index_ = (selected_index_ + 1) % menu_options_.size();
            Resource().setTextButtonHovered(*menu_options_[selected_index_], true);
        }
        else if (kp->code == sf::Keyboard::Key::Enter)
        {
            Audio().playSFX(AudioSystem::SFX::Click);
            executeChoice(choice_map[selected_index_]); 
        }
        else if (kp->code == sf::Keyboard::Key::Escape)
        {
            Audio().playSFX(AudioSystem::SFX::Click);
            showExitPopup();
        }
    }

    if (const auto* mm = event.getIf<sf::Event::MouseMoved>())
    {
        sf::Vector2f mp = context_->window->mapPixelToCoords(mm->position);

        // Hover on center text
        for (size_t i = 0; i < menu_options_.size(); ++i)
        {
            if (menu_options_[i]->getGlobalBounds().contains(mp))
            {
                if (selected_index_ != static_cast<int>(i))
                {
                    Resource().setTextButtonHovered(*menu_options_[selected_index_], false);
                    selected_index_ = static_cast<int>(i);
                    Resource().setTextButtonHovered(*menu_options_[selected_index_], true);
                    Audio().playSFX(AudioSystem::SFX::Hover);
                }
                break;
            }
        }

        // Hover on corner icons
        for (auto& btn : icon_buttons_)
        {
            if (btn.sprite && btn.sprite->getGlobalBounds().contains(mp))
            {
                if (!btn.is_hovered)
                {
                    btn.is_hovered = true;
                    const auto texSize = btn.sprite->getTexture().getSize();
                    btn.sprite->setScale({76.8f / texSize.x, 76.8f / texSize.y});
                    Audio().playSFX(AudioSystem::SFX::Hover);
                }
            }
            else if (btn.is_hovered)
            {
                btn.is_hovered = false;
                const auto texSize = btn.sprite->getTexture().getSize();
                btn.sprite->setScale({64.f / texSize.x, 64.f / texSize.y});
            }
        }
    }

    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mb->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mp = context_->window->mapPixelToCoords(mb->position);

            // Click center text
            for (size_t i = 0; i < menu_options_.size(); ++i)
            {
                if (menu_options_[i]->getGlobalBounds().contains(mp))
                {
                    selected_index_ = static_cast<int>(i);
                    Audio().playSFX(AudioSystem::SFX::Click);
                    executeChoice(choice_map[selected_index_]); 
                    return;
                }
            }

            // Click corner icons
            for (const auto& btn : icon_buttons_)
            {
                if (btn.sprite && btn.sprite->getGlobalBounds().contains(mp))
                {
                    Audio().playSFX(AudioSystem::SFX::Click);
                    executeChoice(btn.choice_index);
                    return;
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// update / render
// ---------------------------------------------------------------------------

void HomeState::update(float dt)
{
    if (coin_text_)
    {
        coin_text_->setString(std::to_string(context_->data->total_coins));
    }
}

void HomeState::render(sf::RenderWindow& window)
{
    if (background_sprite_)
        window.draw(*background_sprite_);

    if (coin_sprite_)
        window.draw(*coin_sprite_);
    if (coin_text_)
        Resource().drawTextWithShadow(window, *coin_text_);

    for (const auto& opt : menu_options_)
        Resource().drawTextWithShadow(window, *opt);

    for (const auto& btn : icon_buttons_)
        if (btn.sprite)
            window.draw(*btn.sprite);

    // Exit confirmation popup
    if (show_exit_popup_)
    {
        if (overlay_bg_)
            window.draw(*overlay_bg_);
        if (popup_bg_)
            window.draw(*popup_bg_);
        if (popup_title_)
            Resource().drawTextWithShadow(window, *popup_title_);
        if (popup_btn_yes_)
            Resource().drawTextWithShadow(window, *popup_btn_yes_);
        if (popup_btn_no_)
            Resource().drawTextWithShadow(window, *popup_btn_no_);
    }
}

// ---------------------------------------------------------------------------
// executeChoice
// ---------------------------------------------------------------------------

void HomeState::executeChoice(int index)
{
    switch (index)
    {
        case 0:
            // std::cout << "Transition to: Playing\n";
            context_->state_machine->push(std::make_unique<PlayingState>(context_));
            break;
        case 1:
            // std::cout << "Transition to: Shop\n";
            context_->state_machine->push(std::make_unique<ShopState>(context_));
            break;
        case 2:
            // std::cout << "Transition to: Load File\n";
            context_->state_machine->push(std::make_unique<LoadState>(context_));
            break;
        case 3:
            // std::cout << "Transition to: Settings\n";
            context_->state_machine->push(std::make_unique<SettingState>(context_));
            break;
        case 4:
            // std::cout << "Transition to: Ranking\n";
            context_->state_machine->push(std::make_unique<RankingState>(context_));
            break;
        case 5:
            // Show popup instead of quitting immediately
            showExitPopup();
            break;
        case 6:
        {
            int vol = (Audio().getSFXVolume() > 0.f) ? 0 : 7;
            Audio().setSFXVolume(vol);
            context_->config->sfx_volume = vol;
            updateSoundIconTextures();
            // std::cout << "SFX volume: " << vol << "\n";
            break;
        }
        case 7:
        {
            int vol = (Audio().getMusicVolume() > 0.f) ? 0 : 5;
            Audio().setMusicVolume(vol);
            context_->config->music_volume = vol;
            updateSoundIconTextures();
            // std::cout << "Music volume: " << vol << "\n";
            break;
        }
        case 8:
        {
            // std::cout << "Transition to: Help\n";
            context_->state_machine->push(std::make_unique<HelpState>(context_));
            break;
        }
        case 9:
        {
            // std::cout << "Transition to: About\n";
            context_->state_machine->push(std::make_unique<AboutState>(context_));
            break;
        }
        case 10:
        {
            // std::cout << "Transition to: Credits\n";
            context_->state_machine->push(std::make_unique<CreditState>(context_));
            break;
        }
    }
}