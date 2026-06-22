#include "../include/entities/animal.h"
#include "../include/world/lane.h"

#pragma region PROTECTED
// pick a random duration for new state between MIN_DURATION_ANIMAL & MAX_DURATION_ANIMAL
float Animal::randomDuration() const
{
	static std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<int> dist(Constants::MIN_DURATION_ANIMAL, Constants::MAX_DURATION_ANIMAL);
	float base = static_cast<float>(dist(rng));
	return base;
}

#pragma endregion

#pragma region PUBLIC
#pragma region CONSTRUCTOR, DESTRUCTOR
//speed, velocity, is_move = 1, timer = 0, duration are calculated & set automactically
//include #randomSpeed(), #randomDuration(), +setSpeed(), +setVelocity(), +setMoveState(), +setDuration()
Animal::Animal(Constants::ObjectType type,
	Constants::ObjectColor color,
	Lane* init_lane)
	: Object(type, color, init_lane)
{
	setSpeed(randomSpeed(type));
	setVelocity({ speed * direction, 0 });
	setMoveState(1); //set timer = 0 & random duration automatically here
}

#pragma endregion

#pragma region GET, SET
float Animal::getSpeed() const
{
	return speed;
}

void Animal::setSpeed(const float& new_speed)
{
	speed = new_speed;
}

float Animal::getTimer() const
{
	return timer;
}

void Animal::setTimer(const float& new_timer)
{
	timer = new_timer;
}

float Animal::getDuration() const
{
	return duration;
}

void Animal::setDuration(const float& new_duration)
{
	duration = new_duration;
}

bool Animal::isMove() const
{
	return is_move;
}

// set new duration and timer = 0 if new state != current state
//include #randomDuration(), +setDuration(), +setTimer()
void Animal::setMoveState(const bool& state)
{
	if (state != is_move)
	{
		is_move = state;
		setDuration(randomDuration());
		setTimer(0.f);
	}
}
#pragma endregion

#pragma region OTHER
//void Animal::playAnimalSound()
//{
//
//}

void Animal::updateState(float delta_time)
{
	if (isOutOfLane() || !is_active)
		return;
	if (timer >= duration)
	{
		setMoveState(!is_move);
	}
	else
	{
		timer += delta_time;
	}
	Animal* nearest_aniaml = current_lane->getNearestAnimalAhead(*this);
	if (nearest_aniaml && is_move)
	{
		float gap = gapToObstacle(nearest_aniaml->getPosition().x, nearest_aniaml->getWidthSprite());
		if (gap <= Constants::VISUAL_DISTANCE * 1.5)
		{
			setMoveState(0);
		}
	}
	if (is_move)
	{
		moveBy(velocity * delta_time);
	}
	setPosition(getPosition());
}

bool Animal::isColliding(const sf::FloatRect& other_hitbox) const
{
	return hitbox.findIntersection(other_hitbox).has_value();
}
#pragma endregion
#pragma endregion