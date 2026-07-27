#pragma once

#include "core/state_machine.h"
#include "states/state.h"
#include "utils/shared_context.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <vector>

class ShopState : public State
{
private:
    std::unique_ptr<sf::Text> title_text_;
    std::unique_ptr<sf::Text> coin_text_;
    std::vector<std::unique_ptr<sf::Text>> menu_texts_;

    int selected_index_;     // 0: Chọn Skin, 1: Nút Mua/Dùng, 2: Back
    int viewing_char_index_; // ID của Skin đang được xem trên màn hình

    sf::Sprite char_sprite_;

    // Dữ liệu giả lập cho 4 nhân vật, sẽ sửa sau (nếu rảnh)
    const int NUM_CHARS = 4;
    const int char_prices_[4] = {0, 50, 100, 200};
    const std::string char_names_[4] = {"DEFAULT", "BLUE SKIN", "GREEN SKIN", "GREY SKIN"};

    void setupUI(const sf::Font &font);
    void updateUI();

    void moveUp();
    void moveDown();
    void nextCharacter();
    void prevCharacter();
    void executeAction();

public:
    ShopState(SharedContext *context);
    ~ShopState() override = default;

    void init() override;
    void handleEvent(const sf::Event &event) override;
    void update(float delta_time) override;
    void render(sf::RenderWindow &window) override;
};