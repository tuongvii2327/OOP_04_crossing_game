#pragma once

#include "states/state.h"
#include "core/state_machine.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class RankingState : public State {
private:
    StateMachine& state_machine_;
    sf::Font font_;
    sf::Text title_text_;
    std::vector<sf::Text> ranking_texts_;

    // Setup ranking display UI
    void setupUI();

public:
    RankingState(StateMachine& state_machine);
    ~RankingState() override = default;

    // Initialize ranking screen with high scores
    void init() override;
    // Handle escape key to exit rankings
    void handleEvent(const sf::Event& event) override;
    // Update ranking state (no-op)
    void update(float delta_time) override;
    // Render ranking display
    void render(sf::RenderWindow& window) override;
};
