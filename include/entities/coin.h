#pragma once

#include "static_object.h"

class Coin : public StaticObject
{
private:
    bool is_collected_ = false;

public:
    Coin(const sf::Vector2f &position = {0.f, 0.f}, Lane *init_lane = nullptr);
    ~Coin() override = default;

    Constants::StaticObjectType getType() const override { return Constants::StaticObjectType::Coin; }
    void updateState(float delta_time) override {} // Xu đứng yên trên làn đường

    bool isCollected() const { return is_collected_; }
    void collect() { is_collected_ = true; }

    void render(sf::RenderWindow &window) const override
    {
        if (!is_collected_)
        {
            StaticObject::render(window); // Chỉ vẽ khi chưa bị ăn
        }
    }
};