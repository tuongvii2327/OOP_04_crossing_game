#pragma once

#include "static_object.h"
#include <algorithm>

class Traffic : public StaticObject
{
private:
    Constants::TrafficLightColor color = Constants::TrafficLightColor::Green;
    float timer;    // time elapsed in the current color
    float duration; // how long the current color lasts

protected:
    float randomDuration(Constants::TrafficLightColor color)
        const; // pick a random duration for the given color (fixed for Yellow)

public:
    // contructor, destructor
    Traffic(Lane* init_lane = nullptr,
            const Constants::TrafficLightColor& init_color = Constants::TrafficLightColor::Green,
            const sf::Vector2f& init_position = sf::Vector2f(0.f, 0.f),
            const float& init_timer = 0.0f);

    // get, set
    Constants::TrafficLightColor getTrafficColor() const;
    void setTrafficColor(const Constants::TrafficLightColor&
                             new_color); // no-op if new_color == current color; resets
                                         // timer/duration and repaints the sprite otherwise

    float getTimer() const;
    void setTimer(const float& new_timer);

    float getDuration() const;
    void setDuration(const float& new_duration);

    void setPosition(const sf::Vector2f& pos) override;

    // Other
    Constants::StaticObjectType getType() const override;
    void updateState(float delta_time) override;
};
