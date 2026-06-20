#pragma once

#include <cmath>
#include "dynamic_object.h"
#include "traffic_light.h"

class Vehicle : public DynamicObject
{
protected:
	//sf::Sound horn_sound;

	float base_speed; //max speed of this vehicle
	float safe_distance; //distance to brake or accelerate

	float calcSafeDistance() const; //safe_distance = base_speed * SAFE_DISTANCE_FACTOR


public:
	//contructor, destructor
	// base_speed, safe_distance, velocity are calculated & set automactically
	Vehicle(const sf::Texture& texture,
		Constants::DynamicObjectType obj_type,
		Lane* init_lane = nullptr);


	//get, set 
	float getBaseSpeed() const;
	void setBaseSpeed(const float& new_speed);

	float getSafeDistance() const;
	//set safe_distance automatically when set base_speed


	//Other
	//void playHornSound();
	void updateState(float delta_time) override;
};
