#pragma once

#include "dynamic_object.h"
#include <SFML/Graphics.hpp>
#include <iostream>

class People : public DynamicObject
{
private:
    bool is_dead;
    float step_size;

    Constants::ObjectColor people_color;

public:
    // Constructors
    People(Constants::ObjectColor color = Constants::ObjectColor::None);
    People(float x, float y, Constants::ObjectColor color = Constants::ObjectColor::None);

    Constants::DynamicObjectType getType() const override;
    Constants::ObjectColor getColor() const override;
    void updateState(float delta_time) override;
    bool isColliding(const sf::FloatRect &other_hitbox) const override;

    bool tryToMove(float dx, float dy, float camera_top_y);

    void die();
    bool isDead() const;
    sf::Vector2f getStartPosition(int init_cell = 6, int init_lane = 10) const;
    void resetPosition();

    sf::FloatRect getBounds() const;
    bool handleEvent(const sf::Event &event, float camera_top_y);

    // ---- Serialization & Deserialzation functions
    void serialize(std::ostream &os) const;
    void deserialize(std::istream &is);
};
