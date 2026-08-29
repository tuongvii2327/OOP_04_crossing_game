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

    // 1. Dùng duy nhất 1 ảnh cỏ cơ bản cho toàn bộ 10 dải nền — init xong gần như tức thì (~1ms).
    //    Các biến thể cỏ còn lại (grass_2 → grass_50) sẽ được tải dần trong initPreloadQueue().
    const float lane_x = static_cast<float>(Constants::GAME_PLAY_BAR_WIDTH);
    const sf::Texture& grass_tex = Resource().getTexture("assets/textures/lane/grass_1.png");
    for (int i = 0; i < 10; ++i)
    {
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

    coin_clock_.restart();
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
    // ── UI Screens ────────────────────────────────────────────────────────────
    textures_to_preload_ = {
        "assets/textures/ui/home.png",
        "assets/textures/ui/shop.png",
        "assets/textures/ui/setting.png",
        "assets/textures/ui/ranking.png",
        "assets/textures/ui/load.png",
        "assets/textures/ui/save.png",
        "assets/textures/ui/sidebar.png",
    };

    // ── Icons ─────────────────────────────────────────────────────────────────
    const std::vector<std::string> icons = {
        "Coin", "Exit", "Gear", "SpeakerOn", "SpeakerMute",
        "music_on", "music_off", "ShoppingCart", "Trophy", "PlayPause"
    };
    for (const auto& icon : icons)
        textures_to_preload_.push_back("assets/textures/icon/" + icon + ".png");

    // ── Lane: Grass ─────────────────────────────────────────────
    for (int i = 1; i <= Constants::getNumberOfTextureVariant(Constants::LaneType::Grass); ++i)
        textures_to_preload_.push_back("assets/textures/lane/grass_" + std::to_string(i) + ".png");

    // ── Lane: Safe ──────────────────────────────────────────────
    for (int i = 1; i <= Constants::getNumberOfTextureVariant(Constants::LaneType::Safe); ++i)
        textures_to_preload_.push_back("assets/textures/lane/safe_" + std::to_string(i) + ".png");

    // ── Lane: Road ───────────────────────────────────────────────
    for (int i = 1; i <= Constants::getNumberOfTextureVariant(Constants::LaneType::Road); ++i)
        textures_to_preload_.push_back("assets/textures/lane/road_" + std::to_string(i) + ".png");

    // ── Lane: River ──────────────────────────────────────────────
    for (int i = 1; i <= Constants::getNumberOfTextureVariant(Constants::LaneType::River); ++i)
        textures_to_preload_.push_back("assets/textures/lane/river_" + std::to_string(i) + ".png");

    // ── Game Entities ─────────────────────────────────────────────────────────

    // People (nhân vật: none, blue, green)
    for (const auto& c : {Constants::ObjectColor::None, Constants::ObjectColor::Blue, Constants::ObjectColor::Green})
        textures_to_preload_.push_back(std::string("assets/textures/people/") + Constants::getObjectColorString(c) + ".png");

    // Car, Truck, Bird, Rabbit (mỗi loại 5 màu)
    for (const auto& entity : {"car", "truck", "bird", "rabbit"})
    {
        for (int c_idx = 0; c_idx <= static_cast<int>(Constants::ObjectColor::None); ++c_idx)
        {
            auto color = static_cast<Constants::ObjectColor>(c_idx);
            textures_to_preload_.push_back(
                std::string("assets/textures/") + entity + "/" + Constants::getObjectColorString(color) + ".png");
        }
    }

    // Wood, Rock (chỉ có none)
    textures_to_preload_.push_back("assets/textures/wood/none.png");
    textures_to_preload_.push_back("assets/textures/rock/none.png");

    // ── Misc ──────────────────────────────────────────────────────────────────
    textures_to_preload_.push_back("assets/textures/traffic/traffic.png");
    textures_to_preload_.push_back("assets/textures/coin/final_spinning_coin.png");

    current_preload_index_ = 0;
}

void LoadingState::updateCoinAnimation(float dt)
{
    if (!coin_sprite_)
        return;

    // Sử dụng đồng hồ thời gian thực (Real-time Clock) độc lập tuyệt đối.
    // Điều này đảm bảo coin xoay mượt mà ngay cả khi Main Thread bị block vài chục ms do load file.
    float real_time = coin_clock_.getElapsedTime().asSeconds();
    int frame_idx = static_cast<int>(real_time / COIN_FRAME_DURATION) % COIN_TOTAL_FRAMES;
    coin_sprite_->setTextureRect(sf::IntRect(
        sf::Vector2i(frame_idx * COIN_FRAME_WIDTH, 0),
        sf::Vector2i(COIN_FRAME_WIDTH, COIN_FRAME_HEIGHT)
    ));
}

void LoadingState::updateProgressBar()
{
    progress_bar_fill_.setSize({progress_bar_bg_.getSize().x * progress_, progress_bar_bg_.getSize().y});

    if (percent_text_)
    {
        int percent = std::clamp(static_cast<int>(progress_ * 100.f), 0, 100);
        percent_text_->setString(std::to_string(percent) + "%");
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

    // 2. Backend: Nạp tài nguyên theo định mức thời gian (Time-budgeted Loading)
    //    Thay vì nạp một số lượng ảnh cố định, chúng ta cho phép nạp liên tục 
    //    nhưng TỐI ĐA 5ms mỗi frame. Điều này ĐẢM BẢO game luôn dư thời gian 
    //    để vẽ (render) đồng xu ở tốc độ mượt mà tuyệt đối 60 FPS (16.6ms/frame).
    sf::Clock frame_budget_clock;
    while (current_preload_index_ < textures_to_preload_.size())
    {
        Resource().getTexture(textures_to_preload_[current_preload_index_]);
        current_preload_index_++;
        
        // Nếu đã nạp quá 5ms trong frame này thì dừng lại để nhường CPU cho việc vẽ đồng xu
        if (frame_budget_clock.getElapsedTime().asMilliseconds() >= 5)
            break;
    }

    if (current_preload_index_ >= textures_to_preload_.size() && !is_done_loading_)
    {
        // Tải âm thanh và khởi động nhạc nền
        Audio().preloadAll();
        // Phải gọi playMusic() trước setMusicVolume() vì sf::Music cần openFromFile() trước khi setVolume()
        Audio().playMusic(AudioSystem::Music::Background);
        if (context_->config)
        {
            Audio().setMusicVolume(context_->config->music_volume);
            Audio().setSFXVolume(context_->config->sfx_volume);
        }
        is_done_loading_ = true;
    }

    // 3. Frontend UX: Thanh tiến trình 3 giai đoạn tâm lý
    //    ┌─────────────────────────────────────────────────────────────────────────┐
    //    │ Phase 1 (0→40%):   0.25s — Chạy nhanh, tạo cảm giác phản hồi tức thì    │
    //    │ Phase 2 (40→80%):  0.65s — Chậm dần (ease-out), xây dựng sự kỳ vọng     │
    //    │ Phase 3 (80→100%): 0.15s — Đợi tải xong, rồi vọt lên 100% dứt khoát     │
    //    └─────────────────────────────────────────────────────────────────────────┘
    loading_timer_ += delta_time;

    constexpr float PHASE1_DURATION = 0.25f; // Thời gian giai đoạn 1 (0% -> 40%)
    constexpr float PHASE2_DURATION = 0.65f; // Thời gian giai đoạn 2 (40% -> 80%)
    constexpr float PHASE3_DURATION = 0.15f; // Thời gian giai đoạn 3 (80% -> 100%)

    if (loading_timer_ <= PHASE1_DURATION)
    {
        // Phase 1: Chạy nhanh từ 0% lên 40% (linear)
        progress_ = (loading_timer_ / PHASE1_DURATION) * 0.40f;
    }
    else if (loading_timer_ <= PHASE1_DURATION + PHASE2_DURATION)
    {
        // Phase 2: Chậm dần từ 40% lên 80% (ease-out)
        float phase_t = (loading_timer_ - PHASE1_DURATION) / PHASE2_DURATION;
        float eased = 1.0f - std::pow(1.0f - phase_t, 1.5f);
        progress_ = 0.40f + 0.40f * eased;
    }
    else
    {
        // Phase 3: Khi đạt mốc 80%
        if (is_done_loading_)
        {
            // Tải thực tế đã xong -> vọt nhanh từ 80% lên 100% trong 0.15s
            float phase3_t = (loading_timer_ - (PHASE1_DURATION + PHASE2_DURATION)) / PHASE3_DURATION;
            progress_ = 0.80f + 0.20f * std::min(phase3_t, 1.0f);
        }
        else
        {
            // Tải thực tế chưa xong -> neo ở 80% chờ backend hoàn tất
            progress_ = 0.80f;
        }
    }

    progress_ = std::clamp(progress_, 0.0f, 1.0f);
    updateProgressBar();

    // 4. Chuyển cảnh khi thanh chạm 100% VÀ tài nguyên đã tải xong thực tế
    if (progress_ >= 1.0f && is_done_loading_ && !transitioned_)
    {
        transitioned_ = true;
        context_->state_machine->switchState(std::make_unique<HomeState>(context_));
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
