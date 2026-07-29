#pragma once

#include "../utils/shared_context.h"
#include "state_machine.h"
#include "states/state.h"
#include <SFML/Graphics.hpp>

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

    void loadPlayerData();
    void savePlayerData();

public:
    Game();
    ~Game();

    void run();
};
