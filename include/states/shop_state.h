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
    std::unique_ptr<sf::Sprite> background_sprite_;
    std::unique_ptr<sf::Text> title_text_;
    std::unique_ptr<sf::Text> coin_text_;
    std::unique_ptr<sf::Sprite> coin_sprite_; // Icon đồng xu (top-left)
    std::vector<std::unique_ptr<sf::Text>> menu_texts_; // [0]=Action, [1]=Back

    // Mũi tên đổi nhân vật (hai bên ảnh)
    std::unique_ptr<sf::Text> arrow_left_;
    std::unique_ptr<sf::Text> arrow_right_;

    int selected_index_;     // 0: Nút Mua/Dùng, 1: Back
    int viewing_char_index_; // ID của Skin đang được xem trên màn hình

    sf::Sprite char_sprite_;

    // Dữ liệu 3 nhân vật
    const int NUM_CHARS = 3;
    const int char_prices_[3] = {0, 50, 100};
    const std::string char_names_[3] = {"PERSON", "ROBOT", "USAGI"};

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