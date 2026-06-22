#pragma once

#include "../utils/constants.h"
#include "../utils/shared_context.h"
#include "state_machine.h"
#include "states/home_state.h"
#include "states/state.h"
#include <SFML/Graphics.hpp>
#include <iostream>

class Game
{
private:
    sf::RenderWindow window;
    StateMachine state_machine;
    sf::Clock clock;

    GameConfig config_;
    GameData data_;
    SharedContext context_;

    // ResourceManager resource_manager;

    void handleEvent();
    void update(float dt);
    void render();

public:
    Game();
    ~Game() = default;

    void run();
};