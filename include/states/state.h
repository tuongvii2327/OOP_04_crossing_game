#pragma once

#include <SFML/Graphics.hpp>

class State
{
public:
    virtual ~State() = default;

    virtual void init() = 0;

    virtual void handleEvent(const sf::Event& event) = 0;

    virtual void update(float delta_time) = 0;
    virtual void render(sf::RenderWindow& window) = 0;

    virtual void pause() {}
    virtual void resume() {}
};