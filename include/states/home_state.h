#pragma once

#include "core/state_machine.h"
#include "states/state.h"
#include "utils/shared_context.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class HomeState : public State
{
private:
    std::unique_ptr<sf::Text> title_text_;
    std::vector<std::unique_ptr<sf::Text>> menu_options_;
    int selected_index_;

    // Format and position a menu option text
    void setupMenuOption(sf::Text &text, const std::string &str,
                         float y_position);
    // Execute action based on selected menu index
    void executeChoice(int index);

public:
    HomeState(SharedContext *context);
    ~HomeState() = default;

    // Initialize home screen UI
    void init() override;
    // Handle keyboard and mouse input
    void handleEvent(const sf::Event &event) override;
    // Update state (no-op for static menu)
    void update(float dt) override;
    // Render menu and title
    void render(sf::RenderWindow &window) override;
};