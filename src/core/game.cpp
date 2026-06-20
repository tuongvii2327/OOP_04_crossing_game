#include "core/game.h"
#include "states/home_state.h"

Game::Game() : window(sf::VideoMode({Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT}), Constants::GAME_TITLE)
{
    window.setFramerateLimit(24); // 24 FPS
    state_machine.addState(std::make_unique<HomeState>(state_machine));
}

void Game::run()
{
    while (window.isOpen())
    {
        state_machine.processStateChanges();
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
    state_machine.getActiveState().render(window);
    window.display();
}