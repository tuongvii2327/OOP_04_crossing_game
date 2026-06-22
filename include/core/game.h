#pragma once

#include <SFML/Graphics.hpp>
#include "state_machine.h"
#include "../utils/constants.h"

class Game
{
private:
	sf::RenderWindow window;
	StateMachine state_machine;
	sf::Clock clock;

	// ResourceManager resource_manager;

	void handleEvent();
	void update(float dt);
	void render();

public:
	Game();
	~Game() = default;

	void run();
};
