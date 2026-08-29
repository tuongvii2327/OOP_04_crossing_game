#include "states/loading_state.h"
#include "states/home_state.h"
#include "utils/audio_system.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"
#include <algorithm>
#include <cmath>

LoadingState::LoadingState(SharedContext* context)
    : State(context), transitioned_(false)
{
}

void LoadingState::init()
{
    initBackground();
    initCoinSprite();
    initUI();
    initPreloadQueue();
}

void LoadingState::initBackground()
{
    grass_lanes_.clear();

    // 1. Ghép 10 dải cỏ từ trên xuống dưới, bắt đầu từ x = GAME_PLAY_BAR_WIDTH (140px)
    const float lane_x = static_cast<float>(Constants::GAME_PLAY_BAR_WIDTH);
    for (int i = 0; i < 10; ++i)
    {
        int variant = ((i * 7) % 50) + 1;
        std::string grass_path = "assets/textures/lane/grass_" + std::to_string(variant) + ".png";

        const sf::Texture& grass_tex = Resource().getTexture(grass_path);
        auto lane = std::make_unique<sf::Sprite>(grass_tex);
        lane->setPosition({lane_x, static_cast<float>(i * Constants::LANE_HEIGHT)});
        grass_lanes_.push_back(std::move(lane));
    }

    // 2. Khởi tạo Sidebar bên trái
    const sf::Texture& sb_tex = Resource().getTexture("assets/textures/ui/sidebar.png");
    sidebar_sprite_ = std::make_unique<sf::Sprite>(sb_tex);
    sidebar_sprite_->setTexture(sb_tex, true);
    sidebar_sprite_->setPosition({0.f, 0.f});
    sidebar_sprite_->setScale({
        static_cast<float>(Constants::WINDOW_WIDTH) / sb_tex.getSize().x,
        static_cast<float>(Constants::WINDOW_HEIGHT) / sb_tex.getSize().y
    });

    // 3. Lớp phủ tối mờ (Dark overlay) phủ lên vùng chơi
    overlay_dim_.setPosition({lane_x, 0.f});
    overlay_dim_.setSize({static_cast<float>(Constants::LANE_WIDTH), static_cast<float>(Constants::WINDOW_HEIGHT)});
    overlay_dim_.setFillColor(sf::Color(0, 0, 0, 120));
}

void LoadingState::initCoinSprite()
{
    const sf::Texture& coin_tex = Resource().getTexture("assets/textures/coin/final_spinning_coin.png");
    coin_sprite_ = std::make_unique<sf::Sprite>(coin_tex);

    current_frame_ = 0;
    coin_sprite_->setTextureRect(sf::IntRect(
        sf::Vector2i(0, 0),
        sf::Vector2i(COIN_FRAME_WIDTH, COIN_FRAME_HEIGHT)
    ));

    // Đặt tâm xoay chính giữa frame 32x32 của đồng xu
    coin_sprite_->setOrigin({COIN_FRAME_WIDTH / 2.f, COIN_FRAME_HEIGHT / 2.f});

    // Scale đồng xu pixel-art to rõ (32px * 4.5 = 144px)
    const float scale_factor = 4.5f;
    coin_sprite_->setScale({scale_factor, scale_factor});

    // Căn giữa khu vực chơi bên phải sidebar (x = 140 + 1260/2 = 770px)
    const float play_area_center_x = Constants::GAME_PLAY_BAR_WIDTH + (Constants::LANE_WIDTH / 2.f);
    coin_sprite_->setPosition({play_area_center_x, Constants::WINDOW_HEIGHT / 2.f - 60.f});
}

void LoadingState::initUI()
{
    const float play_area_center_x = Constants::GAME_PLAY_BAR_WIDTH + (Constants::LANE_WIDTH / 2.f);

    // Chữ thông báo tải tài nguyên
    loading_text_ = std::make_unique<sf::Text>(Resource().createText("LOADING ASSETS...", {44, sf::Color::White}));
    loading_text_->setPosition({play_area_center_x, Constants::WINDOW_HEIGHT / 2.f + 70.f});

    // Chữ phần trăm
    percent_text_ = std::make_unique<sf::Text>(Resource().createText("0%", {32, sf::Color(0xFF, 0xE6, 0x00)}));
    percent_text_->setPosition({play_area_center_x, Constants::WINDOW_HEIGHT / 2.f + 155.f});

    // Khung thanh tiến trình
    const float bar_w = 460.f;
    const float bar_h = 18.f;

    progress_bar_bg_.setSize({bar_w, bar_h});
    progress_bar_bg_.setFillColor(sf::Color(20, 20, 25, 230));
    progress_bar_bg_.setOutlineColor(sf::Color::White);
    progress_bar_bg_.setOutlineThickness(2.f);
    progress_bar_bg_.setOrigin({bar_w / 2.f, bar_h / 2.f});
    progress_bar_bg_.setPosition({play_area_center_x, Constants::WINDOW_HEIGHT / 2.f + 120.f});

    progress_bar_fill_.setSize({0.f, bar_h});
    progress_bar_fill_.setFillColor(sf::Color(0xFF, 0xE6, 0x00));
    progress_bar_fill_.setPosition({progress_bar_bg_.getPosition().x - bar_w / 2.f, progress_bar_bg_.getPosition().y - bar_h / 2.f});
}

void LoadingState::initPreloadQueue()
{
    // Danh sách toàn bộ tài nguyên game cần nạp trước khi vào Main Menu
    textures_to_preload_ = {
        // UI Screens
        "assets/textures/ui/home.png",
        "assets/textures/ui/shop.png",
        "assets/textures/ui/setting.png",
        "assets/textures/ui/ranking.png",
        "assets/textures/ui/load.png",
        "assets/textures/ui/save.png",
        // Icons
        "assets/textures/icon/Coin.png",
        "assets/textures/icon/Exit.png",
        "assets/textures/icon/Gear.png",
        "assets/textures/icon/SpeakerOn.png",
        "assets/textures/icon/SpeakerMute.png",
        "assets/textures/icon/music_on.png",
        "assets/textures/icon/music_off.png",
        "assets/textures/icon/ShoppingCart.png",
        "assets/textures/icon/Trophy.png",
        "assets/textures/icon/PlayPause.png",
        // Game Entities
        "assets/textures/people/none.png",
        "assets/textures/car/none.png",
        "assets/textures/truck/none.png",
        "assets/textures/bird/none.png",
        "assets/textures/rabbit/none.png",
        "assets/textures/wood/none.png",
        "assets/textures/lane/road_1.png",
        "assets/textures/lane/grass_1.png",
        "assets/textures/lane/river_1.png",
        "assets/textures/lane/safe_1.png",
        "assets/textures/traffic/traffic.png"
    };

    current_preload_index_ = 0;
    // +2 cho Audio preload và phát nhạc
    total_tasks_ = textures_to_preload_.size() + 2;
}

void LoadingState::updateCoinAnimation(float dt)
{
    if (!coin_sprite_)
        return;

    anim_timer_ += dt;
    if (anim_timer_ >= COIN_FRAME_DURATION)
    {
        anim_timer_ = 0.f;
        current_frame_ = (current_frame_ + 1) % COIN_TOTAL_FRAMES;
        coin_sprite_->setTextureRect(sf::IntRect(
            sf::Vector2i(current_frame_ * COIN_FRAME_WIDTH, 0),
            sf::Vector2i(COIN_FRAME_WIDTH, COIN_FRAME_HEIGHT)
        ));
    }
}

void LoadingState::updateProgressBar()
{
    progress_bar_fill_.setSize({progress_bar_bg_.getSize().x * progress_, progress_bar_bg_.getSize().y});

    if (percent_text_)
    {
        percent_text_->setString(std::to_string(static_cast<int>(progress_ * 100)) + "%");
        sf::FloatRect bounds = percent_text_->getLocalBounds();
        percent_text_->setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
    }
}

void LoadingState::handleEvent(const sf::Event& /*event*/)
{
    // Chặn input trong lúc loading
}

void LoadingState::update(float delta_time)
{
    // 1. Cập nhật Animation xoay đồng xu
    updateCoinAnimation(delta_time);

    // 2. Nạp dần textures qua từng frame
    if (current_preload_index_ < textures_to_preload_.size())
    {
        Resource().getTexture(textures_to_preload_[current_preload_index_]);
        current_preload_index_++;
    }
    else if (!is_done_loading_)
    {
        // Tải âm thanh và khởi động nhạc nền
        Audio().preloadAll();
        if (context_->config)
        {
            Audio().setMusicVolume(context_->config->music_volume);
            Audio().setSFXVolume(context_->config->sfx_volume);
        }
        Audio().playMusic(AudioSystem::Music::Background);

        current_preload_index_ = total_tasks_;
        is_done_loading_ = true;
    }

    // Tăng dần thanh progress tiến độ mượt mà
    float target_progress = static_cast<float>(current_preload_index_) / static_cast<float>(total_tasks_);
    progress_ += (target_progress - progress_) * 8.f * delta_time;
    if (is_done_loading_ && std::abs(progress_ - 1.0f) < 0.02f)
    {
        progress_ = 1.0f;
    }
    updateProgressBar();

    // 3. Khi hoàn tất 100%, chờ một thoáng rồi chuyển sang HomeState (Main Menu)
    if (progress_ >= 0.999f && !transitioned_)
    {
        finish_delay_ -= delta_time;
        if (finish_delay_ <= 0.f)
        {
            transitioned_ = true;
            context_->state_machine->switchState(std::make_unique<HomeState>(context_));
        }
    }
}

void LoadingState::render(sf::RenderWindow& window)
{
    // 1. Vẽ các dải cỏ phủ kín khu vực lane bên phải (x = 140 -> 1400)
    for (const auto& lane : grass_lanes_)
    {
        if (lane)
            window.draw(*lane);
    }

    // 2. Vẽ lớp phủ tối làm dịu nền cỏ
    window.draw(overlay_dim_);

    // 3. Vẽ Sidebar ở bên trái
    if (sidebar_sprite_)
        window.draw(*sidebar_sprite_);

    // 4. Vẽ đồng xu xoay đè lên background khu vực chơi
    if (coin_sprite_)
        window.draw(*coin_sprite_);

    // 5. Vẽ UI (Text & Thanh tiến trình)
    if (loading_text_)
        ResourceManager::drawTextWithShadow(window, *loading_text_);
    if (percent_text_)
        ResourceManager::drawTextWithShadow(window, *percent_text_);
    window.draw(progress_bar_bg_);
    window.draw(progress_bar_fill_);
}
