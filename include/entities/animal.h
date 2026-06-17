#pragma once

#include "dynamic_object.h"

class Animal : public DynamicObject
{
protected:
	//sf::Sound animal_sound;

	float speed;
	float timer; //time of the current state
	float duration; //max time of the current state
	bool is_move = 0; //current state: 1 = moving, 0 = stop

	float randomDuration() const; // pick a random duration for new state between MIN_DURATION_ANIMAL & MAX_DURATION_ANIMAL

public:
	//contructor, destructor
	//  are calculated & set automactically
	Animal(const sf::Texture& texture,
		Constants::DynamicObjectType type,
		Lane* init_lane = nullptr);


	//get, set 
	float getSpeed() const;
	void setSpeed(const float& new_speed);

	float getTimer() const;
	void setTimer(const float& new_timer);

	float getDuration() const;
	void setDuration(const float& new_duration);

	bool isMove() const;
	void setMoveState(const bool& state); // set new duration and timer = 0 if new state != current state


	//Other
	//void playAnimalSound();
	void updateState(float delta_time) override;
};