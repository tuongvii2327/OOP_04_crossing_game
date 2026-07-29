#include "states/playing_state.h"
#include "../include/utils/serialization.h"
#include "states/game_over_state.h"
#include "states/pause_state.h"
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

void PlayingState::init()
{
    Audio().pauseMusic();
    context_->data->current_score = 0;

    std::cout << "1. Bat dau PlayingState::init()\n";

    const sf::Font& font = Resource().getFont("assets/fonts/VT323-Regular.ttf");
    std::cout << "2. Load Font thanh cong\n";

    score_text_ = std::make_unique<sf::Text>(font);
    score_text_->setString("Score: 0");
    score_text_->setCharacterSize(40);
    score_text_->setFillColor(sf::Color::White);
    score_text_->setOutlineColor(sf::Color::Black);
    score_text_->setOutlineThickness(2.f);
    score_text_->setPosition({20.f, 20.f});

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
    coin_text_ = std::make_unique<sf::Text>(font);
    coin_text_->setString("Coins: " + std::to_string(context_->data->total_coins));
    coin_text_->setCharacterSize(40);
    coin_text_->setFillColor(sf::Color::Yellow);
    coin_text_->setOutlineColor(sf::Color::Black);
    coin_text_->setOutlineThickness(2.f);
    coin_text_->setPosition({20.f, 70.f});

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
        score_text_->setString("Score: " + std::to_string(context_->data->current_score));
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

    if (!context_->config->auto_scroll &&
        (center - target_center) < (Constants::VISUAL_DISTANCE / 5.f))
    {
        center = target_center;
        scroll_velocity_ = 0.f;
    }

    view_.setCenter({view_.getCenter().x, center});
}

void PlayingState::update(float dt)
{
    if (!player_)
        return;
    // 1. NẾU NHÂN VẬT ĐÃ CHẾT -> BẮT ĐẦU ĐẾM GIỜ
    if (player_->isDead())
    {
        death_timer_ += dt;
        if (shake_timer_ > 0.0f)
        {
            shake_timer_ -= dt;
        }

        // Hiệu ứng mờ dần (Fade out) của lớp flash đỏ
        sf::Color flash_color = flash_overlay_.getFillColor();
        if (flash_color.a > 0)
        {
            // Tốc độ tan biến: giảm ~600 alpha mỗi giây
            int new_alpha = flash_color.a - static_cast<int>(600 * dt);
            flash_overlay_.setFillColor(sf::Color(255, 0, 0, std::max(0, new_alpha)));
        }

        // Chờ 0.5 giây để tiếng Crash phát xong
        if (death_timer_ >= 0.5f)
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
        score_text_->setString("Score: " + std::to_string(context_->data->current_score));
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
        coin_text_->setString("Coins: " + std::to_string(context_->data->total_coins));

        // Audio().playSFX(AudioSystem::SFX::Coin);
    }

    // Chỉ kiểm tra va chạm chết người khi player đã đặt chân xuống (không đang nhảy)
    // Khi nhảy, player "đang bay" nên miễn nhiễm — giống Crossy Road gốc
    if ((!player_->isAnimating() && map_.checkLethalCollision(player_->getHitBox())) ||
        (player_->getPosition().y + player_->getHeightSprite() * 0.5f) >= camera_bottom_y)
    {
        Audio().stopAllSFX(); // Dọn sạch mọi tạp âm (tiếng còi xe, tiếng chim...) đang dở dang
        Audio().playSFX(AudioSystem::SFX::Crash);
        player_->die();

        // Kích hoạt rung và nháy đỏ
        shake_timer_ = 0.4f;                                    // Rung mạnh trong 0.4 giây
        flash_overlay_.setFillColor(sf::Color(255, 0, 0, 160)); // Lóe đỏ tức thì (Alpha 160)

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

void PlayingState::render(sf::RenderWindow& window)
{
    if (shake_timer_ > 0.0f)
    {
        sf::View shake_view = view_;
        // Tạo độ lệch ngẫu nhiên từ -8 đến +8 pixels
        float offset_x = static_cast<float>(rand() % 16 - 8);
        float offset_y = static_cast<float>(rand() % 16 - 8);
        shake_view.move({offset_x, offset_y});
        window.setView(shake_view);
    }
    else
        window.setView(view_);
    map_.render(window);

    if (player_)
    {
        player_->render(window);
    }

    window.setView(window.getDefaultView());

    if (flash_overlay_.getFillColor().a > 0)
    {
        window.draw(flash_overlay_);
    }
    if (score_text_)
    {
        window.draw(*score_text_);
    }

    if (coin_text_)
    {
        window.draw(*coin_text_);
    }
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
    score_text_->setString("Score: 0");
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
    score_text_->setString("Score: " + std::to_string(context_->data->current_score));

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
        coin_text_->setString("Coins: " + std::to_string(context_->data->total_coins));
    }
}
