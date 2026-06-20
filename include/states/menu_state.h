#pragma once

#include "core/state_machine.h"
#include "states/state.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class MenuState : public State
{
private:
    StateMachine &state_machine_;
    sf::Font font_;
    std::vector<sf::Text> menu_texts_;
    int selected_index_;

    // Setup and initialize menu UI
    void setupUI();
    // Move selection up with wrap-around
    void moveUp();
    // Move selection down with wrap-around
    void moveDown();

public:
    MenuState(StateMachine &state_machine);
    ~MenuState() override = default;

    // Load font and initialize menu
    void init() override;
    // Handle keyboard navigation and selection
    void handleEvent(const sf::Event &event) override;
    // Update menu state (no-op for static menu)
    void update(float delta_time) override;
    // Render menu options
    void render(sf::RenderWindow &window) override;
};