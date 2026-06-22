#pragma once

#include "object.h"
#include <SFML/Graphics.hpp>

class People : public Object
{
private:
    bool is_dead;
    int score;
    float step_size;

public:
    // Constructors
    People();
    explicit People(const sf::Texture &texture);
    People(float x, float y);

    Constants::ObjectType getType() const override;
    Constants::ObjectColor getColor() const override;
    void updateState(float delta_time) override;
    bool isColliding(const sf::FloatRect &other_hitbox) const override;

    bool tryToMove(float dx, float dy, float camera_top_y);
    void move(float dx, float dy);

    void die();
    bool isDead() const;
    void resetPosition();

    void addScore();
    int getScore() const;

    sf::FloatRect getBounds() const;
};