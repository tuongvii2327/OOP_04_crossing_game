#pragma once
#include "../include/utils/constants.h"
#include <SFML/Graphics.hpp>

class Vehicle;
class Animal;
class Coin;
class Obstacle;

class People
{
private:
    sf::Vector2f position;
    float width, height;
    float step_size;
    bool is_dead; // check if the player is dead
    int score;    // the current score of the player

    sf::RectangleShape shape; // temporarily a shape for test

    // sf::Sprite sprite;          // The actual visual representation
    // sf::Texture texture;        // Holds the image data
    // sf::IntRect current_frame;   // For animation (cropping the sprite sheet)
    // sf::Clock animation_clock;   // To control animation speed (so it's not too fast)
    // float animation_timer = 0.0f;

public:
    People();
    explicit People(const sf::Texture &texture);
    People(float x, float y);

    // move
    void move(float dx, float dy);

    // collision
    sf::FloatRect getBounds() const;
    bool isImpact(const Vehicle *vehicle) const;
    bool isImpact(const Animal *animal) const;
    bool isImpact(const Coin *coin) const;
    bool isImpact(const Obstacle *obstacle) const;

    void addScore(); // increase the score by 1

    // getter
    sf::Vector2f getPosition() const;
    float getX() const;
    float getY() const;
    float getWidth() const;
    float getHeight() const;
    int getScore() const;

    // set player state to dead
    void die();
    void resetPosition();

    // render
    void render(sf::RenderWindow &window);
    void update(float delta_time);
    bool tryToMove(float dx, float dy, float camera_top_y);
};