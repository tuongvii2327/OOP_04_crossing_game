// include/states/setting_state.h
#pragma once

#include "core/state_machine.h"
#include "states/state.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class SettingState : public State
{
private:
    StateMachine &state_machine_;
    sf::Font font_;
    sf::Text title_text_;
    std::vector<sf::Text> setting_texts_;
    int selected_index_;

    int music_volume_;
    int sfx_volume_;
    bool is_hard_mode_;

    // Setup settings menu UI
    void setupUI();
    // Update UI display with current settings
    void updateUI();
    // Move selection up
    void moveUp();
    // Move selection down
    void moveDown();
    // Increase selected setting value
    void increaseSetting();
    // Decrease selected setting value
    void decreaseSetting();

public:
    SettingState(StateMachine &state_machine);
    ~SettingState() override = default;

    // Initialize settings screen
    void init() override;
    // Handle keyboard input for navigation and adjustment
    void handleEvent(const sf::Event &event) override;
    // Update settings state (no-op)
    void update(float delta_time) override;
    // Render settings menu
    void render(sf::RenderWindow &window) override;
};