#pragma once

#include <SFML/Graphics.hpp>

class State
{
public:
    virtual ~State() = default;

    // Initialize state resources and setup
    virtual void init() = 0;

    virtual void handleEvent(const sf::Event& event) = 0;

    virtual void update(float delta_time) = 0;
    virtual void render(sf::RenderWindow& window) = 0;

    // Handle input events
    virtual void handleEvent(const sf::Event &event) = 0;

    // Update game logic with delta time
    virtual void update(float delta_time) = 0;

    // Handle input events
    virtual void handleEvent(const sf::Event &event) = 0;

    // Update game logic with delta time
    virtual void update(float delta_time) = 0;

    // Render state visuals to window
    virtual void render(sf::RenderWindow &window) = 0;

    // Called when state is paused (another state overlaid on top)
    virtual void pause() {}

    // Called when state resumes (overlay removed)
    virtual void resume() {}
};