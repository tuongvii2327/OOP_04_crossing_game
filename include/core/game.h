#pragma once

#include "../utils/shared_context.h"
#include "state_machine.h"
#include "states/state.h"
#include <SFML/Graphics.hpp>

// Class chính điều khiển vòng lặp của toàn bộ trò chơi
class Game
{
private:
    sf::RenderWindow window;
    StateMachine state_machine;
    sf::Clock clock;

    GameConfig config_;
    GameData data_;
    SharedContext context_;

    // TODO: Sẽ tích hợp ResourceManager sau để quản lý tài nguyên (âm thanh, hình ảnh...)

    // Xử lý các sự kiện đầu vào từ người dùng (bàn phím, chuột, đóng cửa sổ)
    void handleEvent();
    
    // Cập nhật logic trò chơi theo thời gian thực dt (delta time)
    void update(float dt);
    
    // Vẽ tất cả các đối tượng của state hiện tại lên cửa sổ
    void render();

    // Tải và lưu dữ liệu cấu hình/tiến trình của người chơi
    void loadPlayerData();
    void savePlayerData();

public:
    Game();
    ~Game();

    // Khởi động vòng lặp chính của trò chơi (Game Loop)
    void run();
};
