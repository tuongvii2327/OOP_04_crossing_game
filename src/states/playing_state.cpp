#include "states/playing_state.h"
#include "states/game_over_state.h"
#include "states/pause_state.h"
#include "states/setting_state.h"
#include "states/home_state.h"
#include "utils/audio_system.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"
#include "utils/serialization.h"
#include <cstdio>
#include <iostream>

PlayingState* PlayingState::active_instance_ = nullptr;

PlayingState::PlayingState(SharedContext* context, int load_slot)
    : State(context), map_(context), load_slot_(load_slot)
{
    active_instance_ = this;
    base_scroll_speed_ = (context_->config->auto_scroll) ? Constants::AUTO_SCROLL_BASE_SPEED : 0.f;
    scroll_velocity_ = 0.f;
    scroll_armed_ = false;
}

PlayingState::~PlayingState()
{
    if (active_instance_ != nullptr)
    {
        active_instance_ = nullptr;
    }
}

// icon_buttons_ layout:
//   [0] Pause
//   [1] Settings 
//   [2] ExitHome  

void PlayingState::init()
{
    Audio().pauseMusic();
    context_->data->current_score = 0;

    std::cout << "1. Bat dau PlayingState::init()\n";

    std::cout << "2. Load Font thanh cong\n";

    score_text_ = std::make_unique<sf::Text>(Resource().createText("Score:\n  0", {40}));
    score_text_->setOrigin({0.f, 0.f});
    score_text_->setPosition({25.f, 20.f});

    if (!context_->data->high_scores.empty())
    {
        current_top_score_ = context_->data->high_scores[0];
    }
    else
    {
        current_top_score_ = 0; // neu khong co high score, > 50 thi duoc chuc mung
    }
    has_beaten_highscore_ = false;
    std::cout << "3. Tao Score Text thanh cong\n";

    // Coin text
    coin_text_ = std::make_unique<sf::Text>(Resource().createText(
        "Coins:\n  " + std::to_string(context_->data->total_coins), {40, sf::Color::Yellow}));
    coin_text_->setOrigin({0.f, 0.f});
    coin_text_->setPosition({25.f, 140.f});

    const sf::Texture& player_tex =
        Resource().getDynamicObjectTexture(Constants::DynamicObjectType::People);
    std::cout << "4. Load Texture Player thanh cong\n";

    Constants::ObjectColor selected_color = Constants::ObjectColor::None;
    switch (context_->data->selected_character)
    {
    case 1:
        selected_color = Constants::ObjectColor::Blue;
        break;
    case 2:
        selected_color = Constants::ObjectColor::Green;
        break;
    default:
        selected_color = Constants::ObjectColor::None;
        break;
    }

    player_ = std::make_unique<People>(selected_color);
    std::cout << "5. Khoi tao Player thanh cong\n";

    // sidebar
    const sf::Texture& sb_tex = Resource().getTexture("assets/textures/ui/sidebar.png");
    sidebar_sprite_ = std::make_unique<sf::Sprite>(sb_tex);
    sidebar_sprite_->setTexture(sb_tex, true);
    sidebar_sprite_->setPosition({ 0.f, 0.f });
    sidebar_sprite_->setScale(
        { static_cast<float>(Constants::WINDOW_WIDTH) / sb_tex.getSize().x,
         static_cast<float>(Constants::WINDOW_HEIGHT) / sb_tex.getSize().y });
    std::cout << "6. Khoi tao Side bar thanh cong\n";

    // Corner icon buttons
    const float x = 60.0f;
    const float by = static_cast<float>(Constants::WINDOW_HEIGHT) - x;
    const float y1 = 340.0f;
    const float y2 = 440.f;

    std::vector<IconConfig> cfgs = {
        // Pause
        {"assets/textures/icon/PlayPause.png", 0, {x, y1}},
        //Setting
        {"assets/textures/icon/Gear.png", 1, {x, y2}},
        // Bottom-Left: Exit
        {"assets/textures/icon/Exit.png", 2, {x, by}}
    };

    icon_buttons_.clear();
    for (const auto& c : cfgs)
    {
        IconButton btn;
        const sf::Texture& tex = Resource().getTexture(c.path);
        btn.sprite = std::make_unique<sf::Sprite>(tex);
        btn.sprite->setTexture(tex, true);
        btn.sprite->setOrigin({ tex.getSize().x / 2.f, tex.getSize().y / 2.f });
        btn.sprite->setPosition(c.pos);
        btn.sprite->setScale({ 4.f, 4.f });
        btn.choice_index = c.choice;
        btn.is_hovered = false;
        icon_buttons_.push_back(std::move(btn));
    }
    std::cout << "7. Khoi tao Buttons thanh cong\n";


    if (load_slot_ > 0)
    {
        std::ifstream file("data/saves/slot_" + std::to_string(load_slot_) + ".bin",
                           std::ios::binary);
        if (file.is_open())
        {
            deserialize(file);
            file.close();
            std::cout << "[PlayingState] Loaded successfully from slot " << load_slot_ << "\n";
        }
        else
        {
            std::cerr << "[PlayingState] Failed to open save file for slot " << load_slot_
                      << "! Starting fresh.\n";
            context_->data->current_score = 0; // Reset điểm về 0
            setupWorld();
            std::cout << "6. Setup World (Map) thanh cong\n";
        }
    }
    else
    {
        context_->data->current_score = 0; // ĐẢM BẢO reset điểm về 0 khi chơi New Game hoàn toàn
        setupWorld();
        std::cout << "6. Setup World (Map) thanh cong\n";
    }

    flash_overlay_.setSize({static_cast<float>(Constants::WINDOW_WIDTH),
                            static_cast<float>(Constants::WINDOW_HEIGHT)});
    flash_overlay_.setFillColor(sf::Color(255, 0, 0, 0));

    
}

void PlayingState::setupWorld()
{
    context_->data->current_level = 0;
    context_->data->current_score = 0;

    map_.reset(Constants::WINDOW_HEIGHT - Constants::LANE_HEIGHT);

    view_.setSize({static_cast<float>(Constants::WINDOW_WIDTH),
                   static_cast<float>(Constants::WINDOW_HEIGHT)});
    view_.setCenter({static_cast<float>(Constants::WINDOW_WIDTH) / 2.f,
                     static_cast<float>(Constants::WINDOW_HEIGHT) / 2.f});
    scroll_velocity_ = 0.f;
    scroll_armed_ = false;
}

void PlayingState::handleEvent(const sf::Event& event)
{
    if (const auto* key_pressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (key_pressed->code == sf::Keyboard::Key::P ||
            key_pressed->code == sf::Keyboard::Key::Escape)
        {
            Audio().playSFX(AudioSystem::SFX::Back);
            context_->state_machine->push(std::make_unique<PauseState>(context_));
            return;
        }
    }

    if (!player_)
        return;

    float cam_top = view_.getCenter().y - (view_.getSize().y / 2.f);
    if (player_->handleEvent(event, cam_top, map_))
    {
        map_.snapPlayerToGrid(player_.get());
        scroll_armed_ = true;
        score_text_->setString("Score:\n  " + std::to_string(context_->data->current_score));
    }


    if (const auto* mm = event.getIf<sf::Event::MouseMoved>())
    {
        sf::Vector2f mp = context_->window->mapPixelToCoords(mm->position);

        // Hover on corner icons
        for (auto& btn : icon_buttons_)
        {
            if (btn.sprite && btn.sprite->getGlobalBounds().contains(mp))
            {
                if (!btn.is_hovered)
                {
                    btn.is_hovered = true;
                    btn.sprite->setScale({ 4.8f, 4.8f });
                    Audio().playSFX(AudioSystem::SFX::Hover);
                }
            }
            else if (btn.is_hovered)
            {
                btn.is_hovered = false;
                btn.sprite->setScale({ 4.f, 4.f });
            }
        }
    }

    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mb->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mp = context_->window->mapPixelToCoords(mb->position);

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

void PlayingState::executeChoice(int index)
{
    switch (index)
    {
    case 0:
        std::cout << "Transition to: Pause\n";
        context_->state_machine->push(std::make_unique<PauseState>(context_));
        break;
    case 1:
        std::cout << "Transition to: Settings\n";
        context_->state_machine->push(std::make_unique<SettingState>(context_));
        break;
    case 2:
        std::cout << "Transition to: Home\n";
        context_->state_machine->push(std::make_unique<HomeState>(context_));
        break;
    }
}

void PlayingState::setBaseScrollSpeed(float new_speed) { base_scroll_speed_ = new_speed; }

float PlayingState::autoScrollSpeed() const
{
    return Constants::AUTO_SCROLL_BASE_SPEED *
           (1.f + static_cast<float>(context_->data->current_level) *
                      Constants::AUTO_SCROLL_LEVEL_FACTOR);
}

void PlayingState::updateCamera(float dt)
{
    float target_center, floor_speed;
    float half_height = view_.getSize().y * 0.5f;
    float center = view_.getCenter().y;
    float player_y = player_->getPosition().y;

    setBaseScrollSpeed(Constants::CAMERA_FOLLOW_LANES_PER_SEC *
                       static_cast<float>(Constants::LANE_HEIGHT));

    if (!context_->config->auto_scroll)
    {
        if (player_y >= center)
        {
            scroll_velocity_ = 0.f;
            return;
        }
        target_center = player_y;
        floor_speed = 0.f;
    }
    else
    {
        if (!scroll_armed_)
        {
            scroll_velocity_ = 0.f;
            return;
        }

        floor_speed = autoScrollSpeed();

        if (scroll_velocity_ <= 0.f)
        {
            scroll_velocity_ = floor_speed;
        }
        target_center = player_y + half_height - Constants::CAMERA_SAFE_DISTANCE;

        if (target_center > center)
        {
            target_center = center;
        }
    }
    float accel = base_scroll_speed_ * base_scroll_speed_ / Constants::CAMERA_SAFE_DISTANCE;
    const float distance = center - target_center;
    const float brake_v = std::sqrt(2.f * accel * std::max(distance, 0.f));
    const float target_v = std::max(brake_v, floor_speed);

    const float max_dv = accel * dt;
    scroll_velocity_ += std::clamp(target_v - scroll_velocity_, -max_dv, max_dv);
    scroll_velocity_ = std::max(scroll_velocity_, floor_speed);

    center -= scroll_velocity_ * dt;
    view_.setCenter({view_.getCenter().x, center});
}

// Kh\u1edfi \u0111\u1ed9ng hi\u1ec7u \u1ee9ng rung m\u00e0n h\u00ecnh v\u1edbi th\u1eddi gian v\u00e0 c\u01b0\u1eddng \u0111\u1ed9 ch\u1ec9 \u0111\u1ecbnh.
// N\u1ebfu \u0111ang rung, ch\u1ec9 ghi \u0111\u00e8 n\u1ebfu c\u01b0\u1eddng \u0111\u1ed9 m\u1edbi m\u1ea1nh h\u01a1n (tr\u00e1nh gi\u1eadt nh\u1eb9 l\u00e0m ghi \u0111\u00e8 rung ch\u1ebft).
void PlayingState::triggerShake(float duration, float intensity)
{
    if (intensity >= shake_intensity_ || shake_timer_ <= 0.f)
    {
        shake_timer_     = duration;
        shake_intensity_ = intensity;
    }
}

void PlayingState::update(float dt)
{
    if (!player_)
        return;

    // Screen Shake: giảm bộ đếm mỗi frame (hoạt động mọi lúc, không chỉ khi chết)
    if (shake_timer_ > 0.0f)
        shake_timer_ -= dt;

    // 1. NẾU NHÂN VẬT ĐÃ CHẾT -> BẮT ĐẦU ĐẾM GIỜ
    if (player_->isDead())
    {
        death_timer_ += dt;

        // Cập nhật hoạt ảnh chết (chìm/teo nhỏ)
        player_->updateState(dt);

        // Hiệu ứng mờ dần (Fade out) của lớp flash
        sf::Color flash_color = flash_overlay_.getFillColor();
        if (flash_color.a > 0)
        {
            // Tốc độ tan biến: giảm ~600 alpha mỗi giây
            int new_alpha = flash_color.a - static_cast<int>(600 * dt);
            flash_overlay_.setFillColor(sf::Color(flash_color.r, flash_color.g, flash_color.b, std::max(0, new_alpha)));
        }

        // Chờ 1.0 giây để xem hiệu ứng chết và tiếng Crash phát xong
        if (death_timer_ >= 1.0f)
        {
            death_timer_ = 0.0f; // Reset lại để dùng cho lần chơi "Try Again"
            context_->state_machine->push(std::make_unique<GameOverState>(context_));
        }
        return; // Đóng băng game, ngừng update map và camera khi đang chờ
    }

    player_->updateState(dt);

    float start_y = player_->getStartPosition().y;
    float current_y = player_->getPosition().y;

    int distance_score = static_cast<int>((start_y - current_y) / Constants::LANE_HEIGHT);

    if (distance_score > context_->data->current_score)
    {
        context_->data->current_score = distance_score;
        score_text_->setString("Score:\n  " + std::to_string(context_->data->current_score));
    }
    context_->data->current_level = context_->data->current_score / 10; // do kho

    updateCamera(dt);

    if (current_top_score_ > 0 && distance_score > current_top_score_ && !has_beaten_highscore_)
    {
        has_beaten_highscore_ = true;                 // Đánh dấu là đã vượt kỷ lục
        Audio().playSFX(AudioSystem::SFX::HighScore); // Phát tiếng ăn mừng
        score_text_->setFillColor(sf::Color::Yellow); // Đổi màu điểm số sang màu Vàng cho ngầu!
    }

    float camera_top_y = view_.getCenter().y - (view_.getSize().y / 2.f);
    float camera_bottom_y = view_.getCenter().y + (view_.getSize().y / 2.f);

    map_.update(camera_top_y, camera_bottom_y, dt);

    // Cập nhật bức tường lửa (auto-scroll)
    updateFireWall(dt, camera_bottom_y);

    // ---River: drift player with Wood they're standing on---
    // Chỉ apply drift khi player đang đứng yên (không nhảy)
    if (!player_->isAnimating())
    {
        sf::Vector2f wood_vel = map_.getWoodVelocityAt(player_->getHitBox());
        if (wood_vel.x != 0.f)
        {
            player_->moveBy({wood_vel.x * dt, 0.f});
        }
    }

    int newly_collected = map_.checkCoinCollision(player_->getHitBox());
    if (newly_collected > 0)
    {
        context_->data->total_coins += newly_collected;
        coin_text_->setString("Coins:\n  " + std::to_string(context_->data->total_coins));

        // Audio().playSFX(AudioSystem::SFX::Coin);
    }

    // Chỉ kiểm tra va chạm chết người khi player đã đặt chân xuống (không đang nhảy)
    // Khi nhảy, player "đang bay" nên miễn nhiễm — giống Crossy Road gốc
    Constants::DeathCause cause_of_death = Constants::DeathCause::None;
    if (!player_->isAnimating())
    {
        cause_of_death = map_.checkLethalCollision(player_->getHitBox());
    }

    if (cause_of_death == Constants::DeathCause::None)
    {
        if ((player_->getPosition().y + player_->getHeightSprite() * 0.5f) >= camera_bottom_y ||
            (player_->getPosition().x + player_->getWidthSprite() * 0.5f) <= Constants::GAME_PLAY_BAR_WIDTH ||
            (player_->getPosition().x + player_->getWidthSprite() * 0.5f) >= Constants::WINDOW_WIDTH)
        {
            cause_of_death = Constants::DeathCause::OutOfBounds;
        }
    }

    if (cause_of_death != Constants::DeathCause::None)
    {
        Audio().stopAllSFX();
        
        if (cause_of_death == Constants::DeathCause::Car)
        {
            Audio().playSFX(AudioSystem::SFX::Crash);
            triggerShake(0.4f, 10.0f);
            flash_overlay_.setFillColor(sf::Color(255, 0, 0, 160)); // Red flash
        }
        else if (cause_of_death == Constants::DeathCause::Drowning)
        {
            // Optional: Audio().playSFX(Splash) if available, for now just use Crash or Jump
            Audio().playSFX(AudioSystem::SFX::Jump);
            triggerShake(0.2f, 5.0f);
            flash_overlay_.setFillColor(sf::Color(0, 100, 255, 160)); // Blue flash
        }
        else if (cause_of_death == Constants::DeathCause::OutOfBounds)
        {
            Audio().playSFX(AudioSystem::SFX::Crash);
            triggerShake(0.3f, 7.0f);
            flash_overlay_.setFillColor(sf::Color(0, 0, 0, 200)); // Black flash
        }

        player_->die(cause_of_death);

        // ---- LOGIC: Xóa sạch file Save trên đĩa khi chết ----
        if (load_slot_ > 0)
        {
            std::string file_path = "data/saves/slot_" + std::to_string(load_slot_) + ".bin";
            std::remove(file_path.c_str()); // Xóa file vật lý khỏi thư mục saves
            std::cout << "[PlayingState] Deleted save file " << file_path
                      << " due to player death.\n";
            load_slot_ = 0; // Đưa trạng thái slot về không
        }
        // context_->state_machine->push(std::make_unique<GameOverState>(context_));
        return;
    }
}

// ============================================================
// BỨC TƯỜNG LỬA (FIRE WALL) - Mép dưới màn hình khi auto-scroll
// ============================================================
void PlayingState::updateFireWall(float dt, float camera_bottom_y)
{
    if (!context_->config->auto_scroll || !scroll_armed_)
        return;

    fire_anim_timer_ += dt;

    // Spawn tàn lửa liên tục
    ember_spawn_timer_ += dt;
    const float spawn_interval = 0.04f; // ~25 hạt/giây
    while (ember_spawn_timer_ >= spawn_interval)
    {
        ember_spawn_timer_ -= spawn_interval;
        Ember e;
        const float r = 2.f + static_cast<float>(std::rand() % 30) / 10.f; // 2~5px
        e.shape.setRadius(r);
        e.shape.setOrigin({r, r});

        // Màu ngẫu nhiên: vàng → cam → đỏ
        int color_roll = std::rand() % 3;
        if (color_roll == 0)
            e.shape.setFillColor(sf::Color(255, 200, 50, 220));  // Vàng sáng
        else if (color_roll == 1)
            e.shape.setFillColor(sf::Color(255, 130, 20, 220));  // Cam rực
        else
            e.shape.setFillColor(sf::Color(255, 60, 10, 200));   // Đỏ thẫm

        float spawn_x = static_cast<float>(Constants::GAME_PLAY_BAR_WIDTH) +
                         static_cast<float>(std::rand() % (Constants::WINDOW_WIDTH - Constants::GAME_PLAY_BAR_WIDTH));
        e.shape.setPosition({spawn_x, camera_bottom_y - static_cast<float>(std::rand() % 40)});
        e.velocity = {(static_cast<float>(std::rand() % 60) - 30.f),
                       -(static_cast<float>(std::rand() % 80) + 40.f)}; // Bay lên trên
        e.lifetime = 0.6f + static_cast<float>(std::rand() % 40) / 100.f;
        e.max_lifetime = e.lifetime;
        embers_.push_back(std::move(e));
    }

    // Cập nhật hạt tàn lửa
    for (auto& e : embers_)
    {
        e.lifetime -= dt;
        e.shape.move(e.velocity * dt);
        float ratio = std::max(0.f, e.lifetime / e.max_lifetime);
        sf::Color col = e.shape.getFillColor();
        e.shape.setFillColor(sf::Color(col.r, col.g, col.b, static_cast<uint8_t>(col.a * ratio)));
        e.shape.setRadius(e.shape.getRadius() * (0.98f + 0.02f * ratio)); // Co nhỏ dần
    }
    embers_.erase(
        std::remove_if(embers_.begin(), embers_.end(),
            [](const Ember& e) { return e.lifetime <= 0.f; }),
        embers_.end());
}

void PlayingState::renderFireWall(sf::RenderWindow& window, float camera_bottom_y)
{
    if (!context_->config->auto_scroll || !scroll_armed_)
        return;

    const float fire_height = 55.f;
    const float fire_base_y = camera_bottom_y - fire_height;
    const float left = static_cast<float>(Constants::GAME_PLAY_BAR_WIDTH);
    const float width = static_cast<float>(Constants::WINDOW_WIDTH - Constants::GAME_PLAY_BAR_WIDTH);



    // --- Vẽ các ngọn lửa bằng tam giác nhấp nhô ---
    const int num_flames = 20;
    const float flame_w = width / static_cast<float>(num_flames);
    for (int i = 0; i < num_flames; ++i)
    {
        float x = left + i * flame_w;
        // Chiều cao ngọn lửa nhấp nhô theo sin với phase offset
        float phase = fire_anim_timer_ * 5.f + static_cast<float>(i) * 1.3f;
        float h = fire_height * (0.6f + 0.4f * std::sin(phase));
        float h2 = fire_height * (0.4f + 0.3f * std::sin(phase * 1.7f + 0.5f));

        // Ngọn lửa chính (cam → vàng)
        sf::ConvexShape flame(3);
        flame.setPoint(0, {x, camera_bottom_y});
        flame.setPoint(1, {x + flame_w, camera_bottom_y});
        flame.setPoint(2, {x + flame_w * 0.5f, camera_bottom_y - h});
        uint8_t green = static_cast<uint8_t>(80 + 100 * std::abs(std::sin(phase * 0.8f)));
        flame.setFillColor(sf::Color(255, green, 0, 200));
        window.draw(flame);

        // Ngọn lửa phụ nhỏ hơn (vàng sáng — lõi lửa)
        sf::ConvexShape inner(3);
        inner.setPoint(0, {x + flame_w * 0.2f, camera_bottom_y});
        inner.setPoint(1, {x + flame_w * 0.8f, camera_bottom_y});
        inner.setPoint(2, {x + flame_w * 0.5f, camera_bottom_y - h2});
        inner.setFillColor(sf::Color(255, 240, 100, 180));
        window.draw(inner);
    }

    // --- Đáy lửa: dải sáng đỏ rực ---
    sf::RectangleShape fire_bottom({width, 8.f});
    fire_bottom.setPosition({left, camera_bottom_y - 8.f});
    fire_bottom.setFillColor(sf::Color(255, 50, 0, 230));
    window.draw(fire_bottom);

    // --- Vẽ tàn lửa (ember particles) ---
    for (const auto& e : embers_)
        window.draw(e.shape);

    // --- Heat Warning: Viền đỏ nhấp nháy khi player gần mép dưới ---
    if (player_ && !player_->isDead())
    {
        float player_bottom = player_->getPosition().y + player_->getHeightSprite();
        float danger_dist = camera_bottom_y - player_bottom;
        float warn_threshold = 2.f * static_cast<float>(Constants::LANE_HEIGHT);
        if (danger_dist < warn_threshold)
        {
            float intensity = 1.0f - (danger_dist / warn_threshold); // 0→1 khi gần sát
            float pulse = 0.5f + 0.5f * std::sin(fire_anim_timer_ * 10.f); // Nhấp nháy
            uint8_t alpha = static_cast<uint8_t>(120 * intensity * pulse);
            sf::RectangleShape warn({width, static_cast<float>(Constants::WINDOW_HEIGHT)});
            warn.setPosition({left, camera_bottom_y - static_cast<float>(Constants::WINDOW_HEIGHT)});
            warn.setFillColor(sf::Color(255, 0, 0, alpha));
            window.draw(warn);
        }
    }
}

void PlayingState::render(sf::RenderWindow& window)
{
    if (shake_timer_ > 0.0f)
    {
        sf::View shake_view = view_;
        // Cường độ rung được kiểm soát bởi shake_intensity_ (nhẹ khi chạm đất, mạnh khi chết)
        int range = static_cast<int>(shake_intensity_) * 2;
        float offset_x = static_cast<float>(rand() % std::max(1, range) - static_cast<int>(shake_intensity_));
        float offset_y = static_cast<float>(rand() % std::max(1, range) - static_cast<int>(shake_intensity_));
        shake_view.move({offset_x, offset_y});
        window.setView(shake_view);
    }
    else
        window.setView(view_);
    map_.render(window);

    // Vẽ bức tường lửa (trước player để lửa nằm phía dưới sprite)
    float cam_bottom = view_.getCenter().y + view_.getSize().y / 2.f;
    renderFireWall(window, cam_bottom);

    if (player_)
    {
        player_->render(window);
    }

    sf::View hud_view(sf::FloatRect(
        {0.f, 0.f},
        {static_cast<float>(Constants::WINDOW_WIDTH), static_cast<float>(Constants::WINDOW_HEIGHT)}
    ));
    window.setView(hud_view);

    if (flash_overlay_.getFillColor().a > 0)
    {
        window.draw(flash_overlay_);
    }

    if (sidebar_sprite_)
    {
        window.draw(*sidebar_sprite_);
    }

    if (score_text_)
    {
        Resource().drawTextWithShadow(window, *score_text_);
    }

    if (coin_text_)
    {
        Resource().drawTextWithShadow(window, *coin_text_);
    }

    for (const auto& btn : icon_buttons_)
        if (btn.sprite)
            window.draw(*btn.sprite);
}

void PlayingState::pause() { std::cout << "[PlayingState] Paused\n"; }

void PlayingState::resume()
{
    Audio().pauseMusic();
    std::cout << "[PlayingState] Resumed\n";
    if (player_ && player_->isDead())
    {
        restart();
    }
}

void PlayingState::restart()
{
    std::cout << "[PlayingState] Restarting Game...\n";

    load_slot_ = 0;
    score_text_->setString("Score:\n  0");
    score_text_->setFillColor(sf::Color::White); // Trả lại màu chữ trắng ban đầu

    if (!context_->data->high_scores.empty())
    {
        current_top_score_ = context_->data->high_scores[0];
    }
    else
    {
        current_top_score_ = 0;
    }
    has_beaten_highscore_ = false;

    death_timer_ = 0.0f;
    shake_timer_ = 0.0f;
    flash_overlay_.setFillColor(sf::Color(255, 0, 0, 0));

    player_->resetPosition();

    setupWorld();
}

// -- LOGIC SERIALIZE CỦA PLAYING STATE ----
void PlayingState::serialize(std::ostream& os, const std::string& slot_name) const
{
    // 1. Lưu điểm số hiện tại của màn chơi
    Serialization::writeString(os, slot_name);
    Serialization::write(os, context_->data->current_score);

    // 2. Lưu tọa độ tâm Camera (sf::View) để khôi phục góc nhìn chính xác
    Serialization::write(os, view_.getCenter().x);
    Serialization::write(os, view_.getCenter().y);

    std::cout << "[DEBUG SAVE] Saving Score: " << context_->data->current_score
              << " | Camera Center Y: " << view_.getCenter().y << "\n";

    // 3. Yêu cầu Player và Map tự lưu dữ liệu của chúng
    player_->serialize(os);
    map_.serialize(os);
}

// ---- LOGIC DESERIALIZE CỦA PLAYING STATE ----
void PlayingState::deserialize(std::istream& is)
{
    // 1. Đọc tên của save slot
    std::string loaded_slot_name;
    Serialization::readString(is, loaded_slot_name);

    // 2. Nạp điểm số và cập nhật lên UI
    Serialization::read(is, context_->data->current_score);
    score_text_->setString("Score:\n  " + std::to_string(context_->data->current_score));

    // 3. Nạp tọa độ tâm Camera
    float cx = 0.f, cy = 0.f;
    Serialization::read(is, cx);
    Serialization::read(is, cy);
    view_.setSize({static_cast<float>(Constants::WINDOW_WIDTH),
                   static_cast<float>(Constants::WINDOW_HEIGHT)});
    view_.setCenter({cx, cy});

    // 4. Yêu cầu Player và Map tự phục hồi dữ liệu
    player_->deserialize(is);
    map_.deserialize(is);

    if (coin_text_)
    {
        coin_text_->setString("Coins:\n  " + std::to_string(context_->data->total_coins));
    }
}
