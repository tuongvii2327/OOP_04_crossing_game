#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

struct GameConfig
{
    int music_volume = 5;
    int sfx_volume = 7;
};

struct GameData
{
    int current_score = 0;
    // max 5 high scores
    std::vector<int> high_scores;
};

class StateMachine; // Forward declaration

struct SharedContext
{
    StateMachine *state_machine;
    sf::RenderWindow *window;
    GameConfig *config;
    GameData *data;
};