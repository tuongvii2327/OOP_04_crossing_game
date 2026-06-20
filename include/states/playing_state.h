#pragma once

#include "core/state_machine.h"
#include "states/state.h"
#include <SFML/Graphics.hpp>
#include <memory>

class PlayingState : public State
{
private:
    StateMachine &state_machine_;

    sf::Font font_;
    std::unique_ptr<sf::Text> score_text_;
    int score_;

    sf::RectangleShape player_;

public:
    PlayingState(StateMachine &state_machine);
    ~PlayingState() = default;

    // Initialize game play resources and player position
    void init() override;
    // Handle player movement and pause input
    void handleEvent(const sf::Event &event) override;
    // Update game physics and logic
    void update(float dt) override;
    // Render player and score
    void render(sf::RenderWindow &window) override;

    // Called when pause menu opens
    void pause() override;
    // Called when pause menu closes
    void resume() override;
};