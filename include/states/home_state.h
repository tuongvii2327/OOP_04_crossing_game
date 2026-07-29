#pragma once

#include "core/state_machine.h"
#include "states/state.h"
#include "utils/shared_context.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class HomeState : public State
{
public:
    struct IconButton
    {
        std::unique_ptr<sf::Sprite> sprite;
        int choice_index;
        bool is_hovered = false;
    };

private:
    std::unique_ptr<sf::Sprite> background_sprite_;
    std::unique_ptr<sf::Text> title_text_;
    std::vector<std::unique_ptr<sf::Text>> menu_options_;
    std::vector<IconButton> icon_buttons_;
    int selected_index_;

    // Coin display (top-left corner)
    std::unique_ptr<sf::Sprite> coin_sprite_;
    std::unique_ptr<sf::Text> coin_text_;

    // Exit confirmation popup
    bool show_exit_popup_ = false;
    int popup_selected_ = 1; // 0 = Yes, 1 = No (mặc định No)
    std::unique_ptr<sf::RectangleShape> overlay_bg_;
    std::unique_ptr<sf::RectangleShape> popup_bg_;
    std::unique_ptr<sf::Text> popup_title_;
    std::unique_ptr<sf::Text> popup_btn_yes_;
    std::unique_ptr<sf::Text> popup_btn_no_;
    bool popup_yes_hovered_ = false;
    bool popup_no_hovered_ = false;

    // Format and position a menu option text
    void setupMenuOption(sf::Text& text, const std::string& str, float y_position);
    // Execute action based on selected menu index
    void executeChoice(int index);
    // Update speaker/music icon textures based on current volume state
    void updateSoundIconTextures();
    // Initialize exit confirmation popup elements
    void initExitPopup(const sf::Font& font);
    // Show exit confirmation popup
    void showExitPopup();

public:
    HomeState(SharedContext* context);
    ~HomeState() = default;

    // Initialize home screen UI
    void init() override;
    // Handle keyboard and mouse input
    void handleEvent(const sf::Event& event) override;
    // Update state
    void update(float dt) override;
    // Render menu and title
    void render(sf::RenderWindow& window) override;
};
