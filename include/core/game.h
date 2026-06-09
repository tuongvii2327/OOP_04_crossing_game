#pragma once

#include <SFML/Graphics.hpp>
#include "state_machine.h"
#include "../utils/constants.h"

class CGame 
{
private:
    sf::RenderWindow mWindow;
    StateMachine mStateMachine;
    sf::Clock mClock;
    
    // ResourceManager mResourceManager; 

    void handleEvent();
    void update(float dt);
    void render();

public:
    CGame();
    ~CGame() = default;

    void run(); 
};