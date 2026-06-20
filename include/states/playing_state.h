#pragma once

#include "core/state_machine.h"
#include "entities/people.h"
#include "states/state.h"
#include "world/map.h"
#include <SFML/Graphics.hpp>
#include <memory>


class PlayingState : public State
{
private:
    StateMachine &state_machine_;

    sf::Font font_;
    std::unique_ptr<sf::Text> score_text_;
    int score_;

    // Tích hợp hệ thống thực thể gốc của bạn vào đây
    People player_;
    Map map_;

    // Camera xử lý cuộn màn hình vô tận theo nhân vật tiến lên
    sf::View view_;

public:
    PlayingState(StateMachine &state_machine);
    ~PlayingState() = default;

    // Khởi tạo tài nguyên UI, vị trí người chơi và sinh bản đồ ban đầu
    void init() override;

    // Xử lý sự kiện nhấn phím di chuyển thông qua tryToMove() của People
    void handleEvent(const sf::Event &event) override;

    // Cập nhật vị trí camera, logic sinh/xóa làn đường của Map
    void update(float dt) override;

    // Render thế giới game song song với giao diện HUD điểm số cố định
    void render(sf::RenderWindow &window) override;

    void pause() override;
    void resume() override;
};