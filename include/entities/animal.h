#pragma once

#include "object.h"

class Animal : public Object
{
protected:
	//sf::Sound animal_sound;

	float speed;
	float timer; // time of the current state
	float duration; // max time of the current state
	bool is_move = 0; // current state: 1 = moving, 0 = stop

	float randomDuration() const; // pick a random duration for new state between MIN_DURATION_ANIMAL & MAX_DURATION_ANIMAL

public:
	// contructor, destructor
	// speed, velocity, is_move = 1, timer = 0, duration are calculated & set automactically
	Animal(Constants::ObjectType type,
		Constants::ObjectColor color,
		Lane* init_lane = nullptr);

	bool isLethal() const override { return true; }
	// get, set
	float getSpeed() const;
	void setSpeed(const float& new_speed);

	float getTimer() const;
	void setTimer(const float& new_timer);

	float getDuration() const;
	void setDuration(const float& new_duration);

	bool isMove() const;
	void setMoveState(const bool& state); // set new duration and timer = 0 if new state != current state


	// Other
	//void playAnimalSound();
	void updateState(float delta_time) override;
	bool isColliding(const sf::FloatRect& other_hitbox) const override;
};
