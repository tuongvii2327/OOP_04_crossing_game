#include "../include/entities/traffic_light.h"
#include "../include/world/lane.h"
#include "../include/utils/resource_manager.h"

#pragma region PROTECTED
//
float Traffic::randomDuration(Constants::TrafficLightColor color) const
{
    if (color == Constants::TrafficLightColor::Yellow)
        return Constants::YELLOW_DURATION;

    static std::mt19937 rng(std::random_device{}());
    
    // SỬA TẠI ĐÂY: Lấy khoảng thời gian động dựa theo màu sắc của đèn
    Constants::DurationRange range = Constants::getDurationRange(color);
    std::uniform_int_distribution<int> dist(range.min_duration, range.max_duration);

    float base = static_cast<float>(dist(rng));
    return base;
}
#pragma endregion

#pragma region PUBLIC

#pragma region CONSTRUCTOR, DESTRUCTOR
//
Traffic::Traffic(Lane *init_lane,
	const Constants::TrafficLightColor &init_color,
	const float &init_timer)
	: Object(Constants::ObjectType::Traffic, Constants::ObjectColor::None, init_lane)
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

Constants::ObjectType Traffic::getType() const
{
	return Constants::ObjectType::Traffic;
}

Constants::ObjectColor Traffic::getColor() const
{
	return Constants::ObjectColor::None;
}

sf::Texture Traffic::getTexture()
{
	return ResourceManager::getTrafficTexture(color);
}
#pragma endregion

#pragma region OTHER
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
	// keep the light vertically locked to its lane as the map scrolls along Oy
	setPosition(getPosition());
}

bool Traffic::isColliding(const sf::FloatRect &other_hitbox) const
{
	return 0;
}
#pragma endregion
#pragma endregion
