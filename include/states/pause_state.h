#pragma once

#include "core/state_machine.h"
#include "states/state.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class PauseState : public State
{
private:
    StateMachine &state_machine_;
    std::unique_ptr<sf::Text> title_text_;
    std::vector<std::unique_ptr<sf::Text>> menu_texts_;
    int selected_index_;

    // Semi-transparent overlay to darken background
    sf::RectangleShape background_overlay_;

    // Setup pause menu UI
    std::unique_ptr<sf::Text> title_text_;

    void setupUI(const sf::Font &font);
    // Move selection up
    void moveUp();
    // Move selection down
    void moveDown();

public:
    PauseState(StateMachine &state_machine);
    ~PauseState() override = default;

    // Load font and initialize pause menu
    void init() override;
    // Handle keyboard input (resume, main menu, escape)
    void handleEvent(const sf::Event &event) override;
    // Update pause state (no-op)
    void update(float delta_time) override;
    // Render pause overlay and menu
    void render(sf::RenderWindow &window) override;
};