#pragma once

#include "core/state_machine.h"
#include "entities/people.h"
#include "states/game_over_state.h"
#include "states/state.h"
#include "utils/resource_manager.h"
#include "utils/shared_context.h"
#include "world/map.h"

#include <SFML/Graphics.hpp>
#include <memory>

class PlayingState : public State
{
private:
    std::unique_ptr<sf::Text> score_text_;
    int score_;

    // Integrate your original entity system here
    std::unique_ptr<People> player_;
    Map map_;

    // Camera handling for infinite scrolling as the player advances
    sf::View view_;
    void setupWorld();

public:
    PlayingState(SharedContext *context);
    ~PlayingState() = default;

    // Initialize UI resources, player position, and generate the initial map
    void init() override;

    // Handle movement key presses via People's tryToMove()
    void handleEvent(const sf::Event &event) override;

    // Update camera position and handle Map's lane spawn/delete logic
    void update(float dt) override;

    // Render the game world alongside the static score HUD overlay
    void render(sf::RenderWindow &window) override;

    void pause() override;
    void resume() override;
    void restart();
};