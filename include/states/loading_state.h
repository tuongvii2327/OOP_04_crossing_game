#pragma once

#include "states/state.h"
#include "utils/shared_context.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>

class LoadingState : public State
{
private:
    // Background & Sidebar giống phong cách game
    std::vector<std::unique_ptr<sf::Sprite>> grass_lanes_;
    std::unique_ptr<sf::Sprite> sidebar_sprite_;
    sf::RectangleShape overlay_dim_;

    // Animation đồng xu từ Spritesheet 360 độ thực sự (192 x 32 -> 6 frames x 32 px)
    std::unique_ptr<sf::Sprite> coin_sprite_;
    sf::Clock coin_clock_; // Đồng hồ độc lập tuyệt đối dành riêng cho đồng xu

    static constexpr int COIN_FRAME_WIDTH = 32;
    static constexpr int COIN_FRAME_HEIGHT = 32;
    static constexpr int COIN_TOTAL_FRAMES = 6;
    static constexpr float COIN_FRAME_DURATION = 0.08f; // Tốc độ xoay mỗi frame

    // UI Loading
    std::unique_ptr<sf::Text> loading_text_;
    std::unique_ptr<sf::Text> percent_text_;
    sf::RectangleShape progress_bar_bg_;
    sf::RectangleShape progress_bar_fill_;

    // Quá trình chuẩn bị tài nguyên và thanh tiến trình
    static constexpr float TARGET_DURATION = 0.7f; // Thời gian hiển thị loading lý tưởng (giây)
    std::vector<std::string> textures_to_preload_;
    size_t current_preload_index_ = 0;
    float loading_timer_ = 0.f;  // Đồng hồ đếm thời gian cho đường cong Ease-Out
    float progress_ = 0.f;
    bool is_done_loading_ = false;
    bool transitioned_ = false;  // Đảm bảo chỉ chuyển sang HomeState 1 lần duy nhất

    void initBackground();
    void initCoinSprite();
    void initUI();
    void initPreloadQueue();
    void updateCoinAnimation(float dt);
    void updateProgressBar();

public:
    explicit LoadingState(SharedContext* context);
    ~LoadingState() override = default;

    void init() override;
    void handleEvent(const sf::Event& event) override;
    void update(float delta_time) override;
    void render(sf::RenderWindow& window) override;
};
