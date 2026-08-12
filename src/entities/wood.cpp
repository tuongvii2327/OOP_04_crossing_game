#include "entities/wood.h"
#include "utils/resource_manager.h"
#include "world/lane.h"
#include "world/lane_river.h"
#include <algorithm>

#pragma region PROTECTED
// safe_distance = base_speed * SAFE_DISTANCE_FACTOR
float Wood::calcSafeDistance() const
{
	return base_speed * Constants::SAFE_DISTANCE_FACTOR;
}
#pragma endregion

#pragma region PUBLIC
#pragma region CONSTRUCTOR, DESTRUCTOR
// Wood floats across the river lane, with base_speed and safe_distance like Vehicle
Wood::Wood(const Constants::ObjectColor &init_color, Lane* init_lane)
	: DynamicObject(Constants::DynamicObjectType::Wood, init_color, init_lane)
{
	wood_color = init_color;
	setBaseSpeed(randomSpeed(Constants::DynamicObjectType::Wood));
	setVelocity({base_speed * direction, 0});

	float target_width =
		static_cast<float>(Constants::getSizeInGame(Constants::DynamicObjectType::Wood).width);
	float scaleX = target_width / sprite.getLocalBounds().size.x;
	float scaleY = Constants::LANE_HEIGHT / sprite.getLocalBounds().size.y;
	sprite.setScale({std::abs(scaleX), std::abs(scaleY)});
	sprite.setOrigin({0.f, 0.f});

	// Hitbox phủ 210px x 105px
	hitbox.size = sf::Vector2f(target_width * Constants::HITBOX_SIZE_RATIO, Constants::LANE_HEIGHT * Constants::HITBOX_SIZE_RATIO);
	hitbox_offset = sf::Vector2f((target_width - hitbox.size.x) / 2.f, (Constants::LANE_HEIGHT - hitbox.size.y) / 2.f);
}
#pragma endregion

#pragma region GET, SET
Constants::ObjectColor Wood::getColor() const
{
	return wood_color;
}

void Wood::setColor(const Constants::ObjectColor &new_color)
{
	wood_color = new_color;
	setSprite(Resource().getDynamicObjectTexture(getType(), new_color), getType());
	sf::Vector2f cur_scale = sprite.getScale();
	sprite.setScale({std::abs(cur_scale.x), std::abs(cur_scale.y)});
}

Constants::DynamicObjectType Wood::getType() const
{
	return Constants::DynamicObjectType::Wood;
}

float Wood::getBaseSpeed() const
{
	return base_speed;
}

// set base_speed & safe_distance
void Wood::setBaseSpeed(const float &new_speed)
{
	base_speed = new_speed;
	safe_distance = calcSafeDistance();
}

float Wood::getSafeDistance() const
{
	return safe_distance;
}
#pragma endregion

#pragma region OTHER
// check if object out of lane, check the nearest wood ahead to reduce current speed, move object
void Wood::updateState(float delta_time)
{
	if (isOutOfLane() || !is_active)
		return;

	sf::Vector2f cur_scale = sprite.getScale();
	if (cur_scale.x < 0.f)
	{
		sprite.setScale({std::abs(cur_scale.x), std::abs(cur_scale.y)});
	}

	float v_wood = base_speed;
	LaneRiver* river = dynamic_cast<LaneRiver*>(current_lane);
	Wood* nearest_wood = river ? river->getNearestWoodAhead(*this) : nullptr;
	if (nearest_wood)
	{
		float gap = gapToObstacle(nearest_wood->getPosition().x, nearest_wood->getWidthSprite());
		float ratio = std::clamp(gap / safe_distance, 0.f, 1.f);
		float nearest_wood_speed = nearest_wood->getVelocity().x * direction;
		v_wood = nearest_wood_speed + ratio * (base_speed - nearest_wood_speed);
	}

	float v_target = std::min(base_speed, v_wood);
	float acceleration = std::pow(base_speed, 2.0) / (safe_distance);
	if (v_target > velocity.x * direction &&
		(velocity.x * direction + acceleration * delta_time) <= base_speed)
	{
		v_target = std::min(v_target, velocity.x * direction + acceleration * delta_time);
	}
	setVelocity({v_target * direction, 0.0f});
	moveBy(velocity * delta_time);
}
#pragma endregion
#pragma endregion
