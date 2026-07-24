#include "../include/entities/vehicle.h"
#include "../include/world/lane.h"
#include "../include/utils/resource_manager.h"

#pragma region PROTECTED
// safe_distance = base_speed * SAFE_DISTANCE_FACTOR
float Vehicle::calcSafeDistance() const
{
	return base_speed * Constants::SAFE_DISTANCE_FACTOR;
}
#pragma endregion

#pragma region PUBLIC

#pragma region CONSTRUCTOR, DESTRUCTOR
// base_speed, safe_distance, velocity are calculated & set automactically
Vehicle::Vehicle(Constants::DynamicObjectType init_type, Constants::ObjectColor init_color, Lane* init_lane)
	: DynamicObject(init_type, init_color, init_lane)
{
	setBaseSpeed(randomSpeed(init_type));     // set safe_distance automatically here
	setVelocity({base_speed * direction, 0}); // direction is already set by DynamicObject
}
#pragma endregion

#pragma region GET, SET
float Vehicle::getBaseSpeed() const
{
	return base_speed;
}

// set base_speed & safe_distance
void Vehicle::setBaseSpeed(const float &new_speed)
{
	base_speed = new_speed;
	safe_distance = calcSafeDistance();
}

float Vehicle::getSafeDistance() const
{
	return safe_distance;
}
#pragma endregion

#pragma region OTHER
// void Vehicle::playHornSound()
// {
// 	horn_sound.play();
// }

// check if object out of map, check the nearest traffic or vehicle to reduce current speed, move object to new position
// check if object out of map, check the nearest traffic or vehicle to reduce current speed, move object to new position
void Vehicle::updateState(float delta_time)
{
	if (isOutOfLane() || !is_active)
		return;
	float v_light = base_speed;
	float v_vehicle = base_speed;
	Traffic* nearest_traffic = current_lane->getNearestTrafficAhead(*this);
	if (nearest_traffic)
	{
		float gap = gapToObstacle(nearest_traffic->getPosition().x,
		                          nearest_traffic->getWidthSprite());
		float ratio = std::clamp(gap / safe_distance, 0.f, 1.f);
		v_light = base_speed * ratio;
	}
	Vehicle* nearest_vehicle = current_lane->getNearestVehicleAhead(*this);
	if (nearest_vehicle)
	{
		float gap = gapToObstacle(nearest_vehicle->getPosition().x,
		                          nearest_vehicle->getWidthSprite());
		float ratio = std::clamp(gap / safe_distance, 0.f, 1.f);
		float nearest_vehicle_speed = nearest_vehicle->getVelocity().x * direction;
		v_vehicle = nearest_vehicle_speed + ratio * (base_speed - nearest_vehicle_speed);
	}
	float v_target = std::min(std::min(base_speed, v_light), v_vehicle);
	float acceleration = std::pow(base_speed, 2.0) / (safe_distance);
	if (v_target > velocity.x * direction && (velocity.x * direction + acceleration * delta_time) <= base_speed)
	{
		v_target = std::min(v_target, velocity.x * direction + acceleration * delta_time);
	}
	setVelocity({v_target * direction, 0.0f});
	moveBy(velocity * delta_time);

	// --- ANIMATION CHO CAR & TRUCK (4 FRAMES: 1 -> 4) ---
	if (std::abs(velocity.x) > 0.1f) // Nếu xe đang chạy
	{
		anim_timer += delta_time;
		if (anim_timer >= 0.1f)
		{
			anim_timer = 0.f;

			if (getType() == Constants::DynamicObjectType::Car)
			{
				current_frame = (current_frame + 1) % 4;
				if (current_frame == 0)
					setSprite(ResourceManager::getInstance().getTexture("assets/textures/car_1.png"));
				else if (current_frame == 1)
					setSprite(ResourceManager::getInstance().getTexture("assets/textures/car_2.png"));
				else if (current_frame == 2)
					setSprite(ResourceManager::getInstance().getTexture("assets/textures/car_3.png"));
				else
					setSprite(ResourceManager::getInstance().getTexture("assets/textures/car_4.png"));
			}
			else if (getType() == Constants::DynamicObjectType::Truck)
			{
				current_frame = (current_frame + 1) % 4;
				if (current_frame == 0)
					setSprite(ResourceManager::getInstance().getTexture("assets/textures/truck_1.png"));
				else if (current_frame == 1)
					setSprite(ResourceManager::getInstance().getTexture("assets/textures/truck_2.png"));
				else if (current_frame == 2)
					setSprite(ResourceManager::getInstance().getTexture("assets/textures/truck_3.png"));
				else
					setSprite(ResourceManager::getInstance().getTexture("assets/textures/truck_4.png"));
			}
		}
	}
}
#pragma endregion
#pragma endregion
