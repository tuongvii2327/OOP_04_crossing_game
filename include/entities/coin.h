#pragma once

#include "static_object.h"

class Wood;
class Coin : public StaticObject
{
private:
    Constants::ObjectColor coin_color;
    bool is_collected_ = false;
    sf::Vector2f velocity_ = {0.f, 0.f};
    Wood* parent_wood_ = nullptr;
    float offset_x_on_wood_ = 0.f;

public:
    Coin(Lane* init_lane = nullptr, const sf::Vector2f& init_position = sf::Vector2f(0.f, 0.f),
         const Constants::ObjectColor& init_color = Constants::ObjectColor::None);

    ~Coin() override = default;

    // Getters
    Constants::StaticObjectType getType() const override;

    void setPosition(const sf::Vector2f& pos) override;
    bool isCollected() const { return is_collected_; }
    void collect() { is_collected_ = true; }

    void setVelocity(const sf::Vector2f& vel) { velocity_ = vel; }
    sf::Vector2f getVelocity() const { return velocity_; }
    void setParentWood(Wood* wood, float offset_x) 
    { 
        parent_wood_ = wood; 
        offset_x_on_wood_ = offset_x;
    }

    // Other
    Constants::ObjectEffect getEffect() const override { return Constants::ObjectEffect::None; }
    void updateState(float delta_time) override;
    void render(sf::RenderWindow& window) const override;
};