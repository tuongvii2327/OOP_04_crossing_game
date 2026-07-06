#include "../include/entities/traffic_light.h"
#include "../include/world/lane.h"
#include "../include/utils/resource_manager.h"

#pragma region PROTECTED
// pick a random duration for the given color; Yellow is always fixed at YELLOW_DURATION
float Traffic::randomDuration(Constants::TrafficLightColor color) const
{
	if (color == Constants::TrafficLightColor::Yellow)
		return Constants::YELLOW_DURATION;

	static std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<int> dist(Constants::MIN_DURATION_TRAFFIC, Constants::MAX_DURATION_TRAFFIC);

	float base = static_cast<float>(dist(rng));
	return base;
}
#pragma endregion

#pragma region PUBLIC

#pragma region CONSTRUCTOR, DESTRUCTOR
// sets the initial color/timer/duration, then repaints the sprite for the very first frame
Traffic::Traffic(Lane* init_lane, const Constants::TrafficLightColor &init_color, const float &init_timer, const sf::Vector2f &init_position)
	: StaticObject(Constants::StaticObjectType::Traffic, init_position, init_lane, Constants::ObjectColor::None)
{
	setTrafficColor(init_color); // set random duration & timer = 0 automatically here
	setTimer(init_timer);
	setDuration(randomDuration(init_color));
	// make sure the lamp shows the right colour from the very first frame
	// (setTrafficColor above skips the sprite update when init_color == default)
	setSprite(ResourceManager::getTrafficTexture(color));
}
#pragma endregion

#pragma region GET, SET
Constants::TrafficLightColor Traffic::getTrafficColor() const
{
	return color;
}

// only acts if new_color != current color: sets color, resets timer, rolls a new duration, and repaints the lamp
void Traffic::setTrafficColor(const Constants::TrafficLightColor &new_color)
{
	if (color != new_color)
	{
		color = new_color;
		setTimer(0.f);
		setDuration(randomDuration(new_color));
		setSprite(ResourceManager::getTrafficTexture(new_color)); // repaint the lamp
	}
}

float Traffic::getTimer() const
{
	return timer;
}

void Traffic::setTimer(const float &new_timer)
{
	timer = new_timer;
}

float Traffic::getDuration() const
{
	return duration;
}

void Traffic::setDuration(const float &new_duration)
{
	duration = new_duration;
}
#pragma endregion

#pragma region OTHER
Constants::StaticObjectType Traffic::getType() const
{
	return Constants::StaticObjectType::Traffic;
}

// cycle Green -> Yellow -> Red -> Green on timeout, then keep the light vertically locked to its lane as the map scrolls along Oy
void Traffic::updateState(float delta_time)
{
	if (timer >= duration)
	{
		if (color == Constants::TrafficLightColor::Green)
			setTrafficColor(Constants::TrafficLightColor::Yellow);
		else if (color == Constants::TrafficLightColor::Yellow)
			setTrafficColor(Constants::TrafficLightColor::Red);
		else if (color == Constants::TrafficLightColor::Red)
			setTrafficColor(Constants::TrafficLightColor::Green);
	}
	else
		timer += delta_time;
	setPosition(getPosition());
}
#pragma endregion
#pragma endregion
