#include "../include/entities/people.h"

#include <SFML/Graphics.hpp>
const int TILE_SIZE = 105;

People::People()
{
    // player's position
    position = sf::Vector2f(Constants::WINDOW_WIDTH / 2.f, Constants::WINDOW_HEIGHT - 50.f);

    // setting game's state
    is_dead = false;
    score = 0;

    shape.setSize(sf::Vector2f(50.0f, 50.0f));
    step_size = 50.0f;
    shape.setFillColor(sf::Color::Green);
    shape.setPosition(position);

    // current_frame = sf::IntRect(0,0,50,50); //Initialize the animation frame

    // sprite.setTexture(texture);
    // sprite.setTextureRect(current_frame); //set the crop
    // sprite.setPosition(position);
}

People::People(float x, float y)
{
    position = sf::Vector2f(x, y);

    is_dead = false;
    score = 0;
    shape.setSize(sf::Vector2f(50.0f, 50.0f));
    shape.setFillColor(sf::Color::Green);
    shape.setPosition(position);
}

void People::move(float dx, float dy)
{
    if (is_dead)
        return;

    position.x += dx;
    position.y += dy;
    shape.setPosition(position);
}

bool People::tryToMove(float dx, float dy, float camera_top_y)
{
    float target_x = position.x + dx;
    float target_y = position.y + dy;

    if (target_x < 0 || target_x >= Constants::WINDOW_WIDTH ||
        target_y < camera_top_y || target_y >= (camera_top_y + Constants::WINDOW_HEIGHT))
        return false;

    move(dx, dy);
    return true;
}

float People::getX() const
{
    return this->position.x;
}

float People::getY() const
{
    return this->position.y;
}

float People::getWidth() const
{
    return this->width;
}

float People::getHeight() const
{
    return this->height;
}

sf::FloatRect People::getBounds() const // return the boundary of the player
{
    return shape.getGlobalBounds();
}

// bool People::isImpact(const Vehicle* vehicle) const
// {
//     if(vehicle == nullptr)
//     {
//          return false;
//     }

//     sf::FloatRect vehicle_rect(vehicle->getX(), vehicle->getY(), vehicle->getWidth(), vehicle->getHeight());
//     return getBounds().intersects(vehicle_rect);
// }

// bool People::isImpact(const Animal* animal) const
// {
//     if(animal == nullptr)
//     {
//          return false;
//     }

//     sf::FloatRect animal_rect(animal->getX(), animal->getY(), animal->getWidth(), animal->getHeight());
//     return getBounds().intersects(animal_rect);
// }

// bool People::isImpact(const Coin* coin) const
// {
//     if(coin == nullptr)
//     {
//          return false;
//     }

//     sf::FloatRect coin_rect(coin->getX(), coin->getY(), coin->getWidth(), coin->getHeight());
//     return getBounds().intersects(coin_rect);
// }

// bool People::isImpact(const Obstacle* obstacle) const
// {
//     if(obstacle == nullptr)
//     {
//          return false;
//     }

//     sf::FloatRect obstacle_rect(obstacle->getX(), obstacle->getY(), obstacle->getWidth(), obstacle->getHeight());
//     return getBounds().intersects(obstacle_rect);
// }

void People::die()
{
    is_dead = true;
}

void People::resetPosition()
{
    // restart at where we start
    position = sf::Vector2f(Constants::WINDOW_WIDTH / 2.f, Constants::WINDOW_HEIGHT - 50.f);
    is_dead = false;
    shape.setPosition(position);

    // sprite.setPosition(position);
}

void People::render(sf::RenderWindow &window)
{
    window.draw(shape);
}

// void People::render(sf::RenderWindow& window){

//     //apply the crop to the sprite
//     sprite.setTextureRect(current_frame);

//     //set position of the sprite
//     sprite.setPosition(position);
//     window.draw(sprite);
// }

void People::update(float delta_time)
{
    if (is_dead)
    {
        return;
    }

    // if(animation_clock.getElapsedTime().asSeconds() > 0.15f) // Check if it's time to cycle the animation frame
    // {
    //     current_frame.left += 50;
    //     if(current_frame.left >= 200) // Loop back to the start if we've passed the last frame
    //     {
    //         current_frame.left = 0;
    //     }

    //     animation_clock.restart();
    // }
}

sf::Vector2f People::getPosition() const
{
    return position;
}

int People::getScore() const
{
    return score;
}

void People::addScore()
{
    score++;
}

People::People(const sf::Texture &texture)
{
    position = sf::Vector2f(Constants::WINDOW_WIDTH / 2.f, Constants::WINDOW_HEIGHT - 50.f);
    is_dead = false;
    score = 0;

    shape.setSize(sf::Vector2f(50.0f, 50.0f));
    shape.setFillColor(sf::Color::Green);
    shape.setPosition(position);
}