#include "../include/core/game.h"
#include <algorithm>
#include <fstream>

Game::Game()
{
	window.create(sf::VideoMode({Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT}),
	              Constants::GAME_TITLE,
	              sf::Style::Titlebar | sf::Style::Close);

	// set FPS for game
	window.setFramerateLimit(Constants::FPS_LIMIT);

	context_.window = &window;
	context_.state_machine = &state_machine;
	context_.config = &config_;
	context_.data = &data_;

	std::ifstream file("data/highscore.bin", std::ios::binary);
	if (file.is_open())
	{
		data_.high_scores.resize(5);
		file.read(reinterpret_cast<char*>(data_.high_scores.data()), 5 * sizeof(int));
		file.close();
	}

	while (data_.high_scores.size() < 5)
	{
		data_.high_scores.push_back(0);
	}

	std::sort(data_.high_scores.begin(), data_.high_scores.end(), std::greater<int>());

	// Start with the HomeState
	state_machine.push(std::make_unique<HomeState>(&context_));
}

void Game::run()
{
	// std::cout << "Game is running..." << std::endl;
	while (window.isOpen())
	{
		// std::cout << "Looping..." << std::endl;
		if (state_machine.processTransitions())
		{
			// Reset clock when a state change occurs to avoid large delta time on the first update of the new state
			clock.restart();
		}

		// If there are no states left, close the window and exit the game loop
		if (state_machine.isEmpty())
		{
			window.close();
			break;
		}

		float dt = clock.restart().asSeconds();
		handleEvent();
		update(dt);
		render();
	}
}

void Game::handleEvent()
{
	while (const std::optional<sf::Event> event = window.pollEvent())
	{
		if (event->is<sf::Event::Closed>())
		{
			window.close();
			return;
		}

		if (!state_machine.isEmpty())
		{
			state_machine.getActiveState().handleEvent(*event);
		}
	}
}

void Game::update(float dt)
{
	state_machine.getActiveState().update(dt);
}

void Game::render()
{
	window.clear(sf::Color::Black);
	state_machine.render(window);
	window.display();
}
