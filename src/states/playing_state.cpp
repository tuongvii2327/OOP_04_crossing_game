#include "states/playing_state.h"
#include "states/game_over_state.h"
#include "states/pause_state.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"
#include <iostream>
#include "../include/utils/serialization.h"

PlayingState* PlayingState::active_instance_ = nullptr;

PlayingState::PlayingState(SharedContext* context, int load_slot)
	: State(context), map_(context), load_slot_(load_slot)
{
	active_instance_ = this;
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
    std::cout << "[PlayingState] Initializing...\n";

    const sf::Font &font = Resource().getFont("assets/fonts/VT323-Regular.ttf");

    score_text_ = std::make_unique<sf::Text>(font);
    score_text_->setString("Score: 0");
    score_text_->setCharacterSize(40);
    score_text_->setFillColor(sf::Color::White);
    score_text_->setOutlineColor(sf::Color::Black);
    score_text_->setOutlineThickness(2.f);
    score_text_->setPosition({20.f, 20.f});

    const sf::Texture &player_tex =
        Resource().getTexture("assets/textures/player.png");
    player_ = std::make_unique<People>(player_tex);

	
    if (load_slot_ > 0)
    {
        // Nếu có yêu cầu load game từ save slot
        std::ifstream file("data/saves/slot_" + std::to_string(load_slot_) + ".bin", std::ios::binary);
        if (file.is_open())
        {
            deserialize(file);
            file.close();
            std::cout << "[PlayingState] Loaded successfully from slot " << load_slot_ << "\n";
        }
        else
        {
            std::cerr << "[PlayingState] Failed to open save file for slot " << load_slot_ << "! Starting fresh.\n";
			context_->data->current_score = 0;
			setupWorld();
        }
    }
    else
    {
		context_->data->current_score = 0;
        setupWorld(); // Tạo thế giới mới từ đầu
    }
}

void PlayingState::setupWorld()
{
    map_.reset(Constants::WINDOW_HEIGHT - Constants::LANE_HEIGHT);

    view_.setSize({static_cast<float>(Constants::WINDOW_WIDTH),
                   static_cast<float>(Constants::WINDOW_HEIGHT)});
    view_.setCenter({static_cast<float>(Constants::WINDOW_WIDTH) / 2.f,
                     static_cast<float>(Constants::WINDOW_HEIGHT) / 2.f});
}

void PlayingState::handleEvent(const sf::Event &event)
{
    if (const auto *key_pressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (key_pressed->code == sf::Keyboard::Key::P ||
            key_pressed->code == sf::Keyboard::Key::Escape)
        {
            context_->state_machine->push(std::make_unique<PauseState>(context_));
            return;
        }
    }

    if (!player_)
        return;

    float cam_top = view_.getCenter().y - (view_.getSize().y / 2.f);
    if (player_->handleEvent(event, cam_top))
    {
        score_text_->setString("Score: " +
                               std::to_string(context_->data->current_score));
    }
}

void PlayingState::update(float dt)
{
    if (!player_ || player_->isDead())
        return;

    player_->updateState(dt);

    float start_y = Constants::WINDOW_HEIGHT - (Constants::LANE_HEIGHT * 1.5f) -
                    (player_->getHeightSprite() / 2.f);
    float current_y = player_->getPosition().y;

    int distance_score =
        static_cast<int>((start_y - current_y) / Constants::LANE_HEIGHT);

    if (distance_score > context_->data->current_score)
    {
        context_->data->current_score = distance_score;
        score_text_->setString("Score: " +
                               std::to_string(context_->data->current_score));
    }

    float current_center_y = view_.getCenter().y;
    if (player_->getPosition().y < current_center_y)
    {
        view_.setCenter({view_.getCenter().x, player_->getPosition().y});
    }

    float camera_top_y = view_.getCenter().y - (view_.getSize().y / 2.f);
    float camera_bottom_y = view_.getCenter().y + (view_.getSize().y / 2.f);

    map_.update(camera_top_y, camera_bottom_y, dt);

    if (map_.checkLethalCollision(player_->getHitBox()) ||
        (player_->getPosition().y + player_->getHeightSprite() * 0.5f) >= camera_bottom_y)
    {
        player_->die();

		// ---- LOGIC: Xóa sạch file Save trên đĩa khi chết ----
        if (load_slot_ > 0)
        {
            std::string file_path = "data/saves/slot_" + std::to_string(load_slot_) + ".bin";
            std::remove(file_path.c_str()); // Xóa file vật lý khỏi thư mục saves
            std::cout << "[PlayingState] Deleted save file " << file_path << " due to player death.\n";
            load_slot_ = 0; // Đưa trạng thái slot về không
        }
        context_->state_machine->push(std::make_unique<GameOverState>(context_));
        return;
    }
}

void PlayingState::render(sf::RenderWindow &window)
{
    window.setView(view_);
    map_.render(window);

    if (player_)
    {
        player_->render(window);
    }

    window.setView(window.getDefaultView());
    if (score_text_)
    {
        window.draw(*score_text_);
    }
}

void PlayingState::pause()
{
    std::cout << "[PlayingState] Paused\n";
}

void PlayingState::resume()
{
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
    context_->data->current_score = 0;
    score_text_->setString("Score: 0");

    player_->resetPosition();

    setupWorld();
}

// -- LOGIC SERIALIZE CỦA PLAYING STATE ----
void PlayingState::serialize(std::ostream &os, const std::string &slot_name) const
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
void PlayingState::deserialize(std::istream &is)
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
    view_.setSize({static_cast<float>(Constants::WINDOW_WIDTH), static_cast<float>(Constants::WINDOW_HEIGHT)});
    view_.setCenter({cx, cy});

    // 4. Yêu cầu Player và Map tự phục hồi dữ liệu
    player_->deserialize(is);
    map_.deserialize(is);
}
