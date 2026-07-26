#pragma once

#include "dynamic_object.h"

class Vehicle : public DynamicObject
{
protected:
	// sf::Sound horn_sound;

	float base_speed;    // max speed of this vehicle
	float safe_distance; // distance to brake or accelerate

	float calcSafeDistance() const; // safe_distance = base_speed * SAFE_DISTANCE_FACTOR

public:
	// contructor, destructor
	Vehicle(const Constants::DynamicObjectType &init_type, 
		const Constants::ObjectColor &init_color, 
		Lane* init_lane = nullptr); // base_speed, safe_distance, velocity are calculated & set automactically

	// get, set
	float getBaseSpeed() const;
	void setBaseSpeed(const float &new_speed);

	float getSafeDistance() const;
	// set safe_distance automatically when set base_speed

	// Other
	void playHornSound();
	Constants::ObjectEffect getEffect() const override
	{
		return Constants::ObjectEffect::Lethal;
	}
	void updateState(float delta_time) override;
};
