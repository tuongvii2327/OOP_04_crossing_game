#include "states/playing_state.h"
#include "states/pause_state.h"
#include "utils/constants.h"
#include <iostream>

PlayingState::PlayingState(StateMachine &state_machine)
    : state_machine_(state_machine), score_(0) {}

void PlayingState::init()
{
    std::cout << "[PlayingState] Initializing...\n";

    if (!font_.openFromFile("assets/fonts/VT323-Regular.ttf"))
    {
        std::cerr << "Error: Cannot load font in PlayingState!\n";
    }

    score_text_ = std::make_unique<sf::Text>(font_);
    score_text_->setString("Score: 0");
    score_text_->setCharacterSize(40);
    score_text_->setFillColor(sf::Color::White);
    score_text_->setOutlineColor(sf::Color::Black);
    score_text_->setOutlineThickness(2.f);
    score_text_->setPosition({20.f, 20.f});

    // 1. Khởi tạo trạng thái nhân vật từ class People gốc
    player_.resetPosition();

    // 2. Khởi tạo bản đồ nền phát triển ngược lên trên xuất phát từ đáy màn hình
    map_.initMap(Constants::WINDOW_HEIGHT - Constants::LANE_HEIGHT);

    // 3. Cấu hình Camera góc nhìn mặc định ban đầu khớp với kích thước cửa sổ game
    view_.setSize({static_cast<float>(Constants::WINDOW_WIDTH), static_cast<float>(Constants::WINDOW_HEIGHT)});
    view_.setCenter({static_cast<float>(Constants::WINDOW_WIDTH) / 2.f, static_cast<float>(Constants::WINDOW_HEIGHT) / 2.f});
}

void PlayingState::handleEvent(const sf::Event &event)
{
    if (const auto *key_pressed = event.getIf<sf::Event::KeyPressed>())
    {
        // Nhấn ESC hoặc P để mở menu Tạm dừng dạng đè màn hình (Overlay)
        if (key_pressed->code == sf::Keyboard::Key::P || key_pressed->code == sf::Keyboard::Key::Escape)
        {
            state_machine_.addState(std::make_unique<PauseState>(state_machine_), false);
            return;
        }

        // Bước di chuyển khớp với step_size định nghĩa trong class People (50.0f)
        float step = 50.f;

        if (key_pressed->code == sf::Keyboard::Key::W || key_pressed->code == sf::Keyboard::Key::Up)
        {
            // Sử dụng hàm kiểm tra biên tryToMove của bạn
            if (player_.tryToMove(0.f, -step))
            {
                player_.addScore();
                score_ = player_.getScore();
                score_text_->setString("Score: " + std::to_string(score_));
            }
        }
        else if (key_pressed->code == sf::Keyboard::Key::S || key_pressed->code == sf::Keyboard::Key::Down)
        {
            player_.tryToMove(0.f, step);
        }
        else if (key_pressed->code == sf::Keyboard::Key::A || key_pressed->code == sf::Keyboard::Key::Left)
        {
            player_.tryToMove(-step, 0.f);
        }
        else if (key_pressed->code == sf::Keyboard::Key::D || key_pressed->code == sf::Keyboard::Key::Right)
        {
            player_.tryToMove(step, 0.f);
        }
    }
}

void PlayingState::update(float dt)
{
    // Cập nhật hoạt ảnh/logic của người chơi
    player_.update(dt);

    // Tính toán Camera: Khi người chơi tiến lên quá nửa màn hình, camera sẽ đẩy lên theo
    float current_center_y = view_.getCenter().y;
    if (player_.getPosition().y < current_center_y)
    {
        view_.setCenter({view_.getCenter().x, player_.getPosition().y});
    }

    // Lấy tọa độ biên trên và biên dưới thực tế của camera góc nhìn hiện tại
    float camera_top_y = view_.getCenter().y - (view_.getSize().y / 2.f);
    float camera_bottom_y = view_.getCenter().y + (view_.getSize().y / 2.f);

    // Cập nhật Map: Sinh thêm hàng ở đỉnh, xóa hàng khuất ở đáy, đồng thời cho xe chạy
    // BẮT BUỘC: Bạn cần sửa hàm map_.update nhận thêm tham số dt (Xem hướng dẫn mục 3 bên dưới)
    map_.update(camera_top_y, camera_bottom_y, dt);
}

void PlayingState::render(sf::RenderWindow &window)
{
    // BƯỚC 1: Áp dụng góc nhìn camera di động để vẽ thế giới game (Đường xá, xe cộ, người chơi)
    window.setView(view_);
    map_.render(window);
    player_.render(window);

    // BƯỚC 2: Trả cửa sổ về góc nhìn mặc định cố định để vẽ UI/HUD (Điểm số giữ nguyên vị trí trên màn hình)
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
}