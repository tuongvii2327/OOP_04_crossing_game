// include/states/save_state.h
#pragma once

#include "core/state_machine.h"
#include "states/state.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class SaveState : public State
{
private:
    StateMachine &state_machine_;
    sf::Font font_;
    sf::Text title_text_;
    std::vector<sf::Text> save_slot_texts_;
    int selected_index_;
    bool is_saved_ = false;
    float save_timer_ = 0.0f;

    // Setup save slot UI
    void setupUI();
    // Move selection up
    void moveUp();
    // Move selection down
    void moveDown();
    // Save game data to specified slot
    void saveGameData(int slot_index);

public:
    SaveState(StateMachine &state_machine);
    ~SaveState() override = default;

    // Initialize save screen
    void init() override;
    // Handle slot selection and navigation
    void handleEvent(const sf::Event &event) override;
    // Update save state (no-op)
    void update(float delta_time) override;
    // Render save menu
    void render(sf::RenderWindow &window) override;
};