#pragma once

#include "core/state_machine.h"
#include "states/state.h"
#include "utils/resource_manager.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class RankingState : public State
{
private:
    StateMachine &state_machine_;
    std::unique_ptr<sf::Text> title_text_;
    std::vector<std::unique_ptr<sf::Text>> ranking_texts_;

    // Setup ranking display UI
    void setupUI(const sf::Font &font);

public:
    RankingState(StateMachine &state_machine);
    ~RankingState() override = default;

    // Initialize ranking screen with high scores
    void init() override;
    // Handle escape key to exit rankings
    void handleEvent(const sf::Event &event) override;
    // Update ranking state (no-op)
    void update(float delta_time) override;
    // Render ranking display
    void render(sf::RenderWindow &window) override;
};
