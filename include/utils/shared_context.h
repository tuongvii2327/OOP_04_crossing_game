#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

struct GameConfig
{
	int music_volume = 5;
	int sfx_volume = 7;
	bool auto_scroll = 0;
};

struct GameData
{
	int current_score = 0;
	int current_level = 0;
	int total_coins = 0;
	// max 5 high scores
	std::vector<int> high_scores;

	// for shop
	int selected_character = 0;
	std::vector<bool> unlocked_characters = {true, false, false, false};
};

class StateMachine; // Forward declaration

struct SharedContext
{
	StateMachine* state_machine;
	sf::RenderWindow* window;
	GameConfig* config;
	GameData* data;
};