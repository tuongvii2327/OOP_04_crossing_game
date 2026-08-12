#include "entities/traffic_light.h"
#include "utils/resource_manager.h"
#include "world/lane.h"

#pragma region PROTECTED
// pick a random duration for the given color; Yellow is always fixed at YELLOW_DURATION
float Traffic::randomDuration(Constants::TrafficLightColor color) const
{
	if (color == Constants::TrafficLightColor::Yellow)
		return Constants::YELLOW_DURATION;

	static std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<int> dist(Constants::MIN_DURATION_TRAFFIC,
		Constants::MAX_DURATION_TRAFFIC);

	float base = static_cast<float>(dist(rng));
	return base;
}
#pragma endregion

#pragma region PUBLIC

#pragma region CONSTRUCTOR, DESTRUCTOR
// sets the initial color/timer/duration, then repaints the sprite for the very first frame
Traffic::Traffic(Lane* init_lane, const Constants::TrafficLightColor &init_color, const sf::Vector2f &init_position, const float &init_timer)
	: StaticObject(init_lane, Constants::StaticObjectType::Traffic, init_position, Constants::ObjectColor::None)
{
	color = init_color;
	setTimer(init_timer);
	setDuration(randomDuration(init_color));
	const sf::Texture &tex = Resource().getTrafficTexture(color);
	sprite.setTexture(tex, false);
	sprite.setTextureRect(Resource().getTrafficTextureRect(color));
	adjustSprite(Constants::StaticObjectType::Traffic);
	setDirection(direction);
	setHitbox();
}
#pragma endregion

#pragma region GET, SET
Constants::TrafficLightColor Traffic::getTrafficColor() const
{
	return color;
}

// only acts if new_color != current color: sets color, resets timer, rolls a new duration, and
// repaints the lamp
void Traffic::setTrafficColor(const Constants::TrafficLightColor &new_color)
{
	if (color != new_color)
	{
		color = new_color;
		setTimer(0.f);
		setDuration(randomDuration(new_color));
		const sf::Texture &tex = Resource().getTrafficTexture(color);
		sprite.setTexture(tex, false);
		sprite.setTextureRect(Resource().getTrafficTextureRect(color));
		adjustSprite(Constants::StaticObjectType::Traffic);
		setDirection(direction);
		setHitbox();
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

void Traffic::setPosition(const sf::Vector2f &pos)
{
	position = pos;
	if (current_lane != nullptr)
	{
		position.y = current_lane->getPosition().y - Constants::TRAFFIC_LIGHT_Y_OFFSET;
	}
	sprite.setPosition(position);
	hitbox.position = position + hitbox_offset; // update hitbox
}
#pragma endregion

#pragma region OTHER
Constants::StaticObjectType Traffic::getType() const
{
	return Constants::StaticObjectType::Traffic;
}

// cycle Green -> Yellow -> Red -> Green on timeout, then keep the light vertically locked to its
// lane as the map scrolls along Oy
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
