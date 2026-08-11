#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

// Cấu hình của trò chơi, bao gồm âm thanh và các cài đặt khác
struct GameConfig
{
    int music_volume = 5; // Âm lượng nhạc nền (0-100)
    int sfx_volume = 7;   // Âm lượng hiệu ứng âm thanh (0-100)
    bool auto_scroll = 0; // Chế độ tự động cuộn bản đồ
};

// Dữ liệu tiến trình của người chơi (có thể lưu trữ và tải lại)
struct GameData
{
    int current_score = 0; // Điểm hiện tại trong màn chơi
    int current_level = 0; // Độ khó/Level hiện tại
    int total_coins = 0;   // Tổng số tiền thu thập được

    // Lưu trữ danh sách 5 điểm số cao nhất của người chơi
    std::vector<int> high_scores;

    // Dữ liệu dùng cho cửa hàng (Shop)
    int selected_character = 0; // ID nhân vật đang chọn
    std::vector<bool> unlocked_characters = {true, false, false}; // Trạng thái mở khóa của các nhân vật
};

class StateMachine; // Khai báo trước để tránh vòng lặp include

// Context dùng chung để truyền dữ liệu giữa các state (Màn hình)
struct SharedContext
{
    StateMachine* state_machine;
    sf::RenderWindow* window;
    GameConfig* config;
    GameData* data;
};