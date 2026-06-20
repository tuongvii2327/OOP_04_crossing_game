#include "people.h"
#include <SFML/Graphics.hpp>


const int TILE_SIZE = 105;
const int GRID_WIDTH = 12;  // 12 columns
const int GRID_HEIGHT = 10; // 10 rows
const int SCREEN_WIDTH = 1400;
const int SCREEN_HEIGHT = 1050;
const int SIDEBAR_WIDTH = 140;

People::People(){
    //player's position
    position = sf::Vector2f(0.0f, 0.0f);

    //setting the rectangle's size
    width = 50.0f;
    height = 50.0f;

    //setting game's state
    is_dead = false;
    score = 0;

    shape.setSize(sf::Vector2f(50.0f, 50.0f)); 
    shape.setFillColor(sf::Color::Green);      
    shape.setPosition(position);

    // current_frame = sf::IntRect(0,0,50,50); //Initialize the animation frame

    // sprite.setTexture(texture);
    // sprite.setTextureRect(current_frame); //set the crop
    // sprite.setPosition(position);

}

void People::move(float dx, float dy)
{
    if(is_dead)
    {
        return;
    }

    position.x += dx;
    position.y += dy;

    if(position.x < 0)
    {
        position.x = 0;
    }
    else if(position.x > GRID_WIDTH*TILE_SIZE)
    {
        position.x = GRID_WIDTH*TILE_SIZE;
    }

    if(position.y < 0)
    {
        position.y = 0;
    }
    else if(position.y > GRID_HEIGHT*TILE_SIZE)
    {
        position.y = GRID_HEIGHT*TILE_SIZE;
    }

    shape.setPosition(position);
}

sf::Vector2f People::getPosition() const
{
    return this->position;
}

float People::getWidth() const
{
    return this->width;
}

float People::getHeight() const
{
    return this->height;
}

int People::getScore() const
{
    return this->score;
}

sf::FloatRect People::getBounds() const //return the boundary of the player
{
    return sf::FloatRect(position.x,position.y,width,height);
}

bool People::isImpact(const Vehicle* vehicle) const  //check if collision happens for vehicle
{
    if(vehicle == nullptr)
    {
         return false;
    }

    sf::Vector2f vehicle_position = vehicle->getPosition();
    sf::FloatRect vehicle_rect(vehicle_position.x, vehicle_position.y, vehicle->getWidth(), vehicle->getHeight());
    return getBounds().intersects(vehicle_rect);
}

bool People::isImpact(const Animal* animal) const //check if collision happens for animal
{
    if(animal == nullptr)
    {
         return false;
    }

    sf::Vector2f animal_position = animal->getPosition();
    sf::FloatRect animal_rect(animal_position.x, animal_position.y, animal->getWidth(), animal->getHeight());
    return getBounds().intersects(animal_rect);
}

bool People::isImpact(const Coin* coin) const //check if collision happens for coin
{
    if(coin == nullptr)
    {
         return false;
    }

    sf::Vector2f coin_position = coin->getPosition();
    sf::FloatRect coin_rect(coin_position.x, coin_position.y, coin->getWidth(), coin->getHeight());
    return getBounds().intersects(coin_rect);
}

bool People::isImpact(const Obstacle* obstacle) const //check if collision happens for obstacle
{
    if(obstacle == nullptr)
    {
         return false;
    }

    sf::Vector2f obstacle_position = obstacle->getPosition();
    sf::FloatRect obstacle_rect(obstacle_position.x, obstacle_position.y, obstacle->getWidth(), obstacle->getHeight());
    return getBounds().intersects(obstacle_rect);
}

void People::addScore(){
    score += 1;
}

void People::die()
{
    is_dead = true;
}

void People::resetPosition(){
    //restart at where we start
    position = sf::Vector2f(0.0f, 0.0f); //reset position
    is_dead = false; //reset the state
    shape.setPosition(position);

    // sprite.setPosition(position);
}

void People::render(sf::RenderWindow& window) {
    window.draw(shape); //update the position of the character on screen
}

// void People::render(sf::RenderWindow& window){

//     //apply the crop to the sprite
//     sprite.setTextureRect(current_frame);

//     //set position of the sprite
//     sprite.setPosition(position);
//     window.draw(sprite);
// }

// void People::update(float delta_time){
//     if(is_dead)
//     {
//         return;
//     }

//     if(animation_clock.getElapsedTime().asSeconds() > 0.15f) // Check if it's time to cycle the animation frame
//     {
//         current_frame.left += 50;
//         if(current_frame.left >= 200) // Loop back to the start if we've passed the last frame
//         {
//             current_frame.left = 0;
//         }

//         animation_clock.restart();
//     }
// }

