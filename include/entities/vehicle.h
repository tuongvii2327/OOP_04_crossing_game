#pragma once

#include <cmath>
#include "dynamic_object.h"
#include "traffic_light.h"

class Vehicle : public DynamicObject
{
protected:
#pragma region VARIABLES
	//sf::Sound horn_sound;

	float base_speed; //fixed for the object's lifetime (px/s)
	float safe_distance; //braking & following distance (px)
	Constants::MotionState motion_state;
	Constants::LightDecision light_decision; //latched decision for the current light phase
#pragma endregion

	float calcBaseSpeed(Constants::DynamicObjectType type) const; //randomSpeed for type, scaled by lane level
	float calcSafeDistance() const; //safe_distance = base_speed * SAFETY_FACTOR



public:
	//contructor, destructor
	Vehicle(const sf::Texture& texture,
		Constants::DynamicObjectType obj_type,
		Lane* init_lane = nullptr);


	//get, set 
	float getBaseSpeed() const;
	void setBaseSpeed(const float& newSpeed);

	float getSafeDistance() const;
	//set safe_distance automatically when set base_speed

	Constants::MotionState getMotionState() const;
	void setMotionState(const Constants::MotionState& state);

	Constants::LightDecision getLightDecision() const;
	void setLightDecision(const Constants::LightDecision& decision);


	//Save/Load
	//void serialize(std::ostream& outDev) const override;  //write state of object to binary file
	//void deserialize(std::istream& inDev) override; //read state of object from binary file


	//Other
	//void playHornSound();
	void updateState(float delta_time) override;
};
