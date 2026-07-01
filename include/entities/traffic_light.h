#pragma once

#include "object.h"
#include <algorithm>

class Traffic : public Object
{
private:
	Constants::TrafficLightColor color = Constants::TrafficLightColor::Green;
	float timer;
	float duration;

protected:
	float randomDuration(Constants::TrafficLightColor color) const;

public:
	// contructor, destructor
	Traffic(Lane* init_lane = nullptr, const Constants::TrafficLightColor &init_color = Constants::TrafficLightColor::Green, const float &init_timer = 0.0f);

	// get, set
	Constants::TrafficLightColor getTrafficColor() const;
	void setTrafficColor(const Constants::TrafficLightColor &new_color);

	float getTimer() const;
	void setTimer(const float &new_timer);

	float getDuration() const;
	void setDuration(const float &new_duration);

	Constants::ObjectType getType() const override;
	Constants::ObjectColor getColor() const override;
	sf::Texture getTexture();

	// Other
	void updateState(float delta_time) override;
	bool isColliding(const sf::FloatRect &other_hitbox) const override;
};
