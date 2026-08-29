#include "entities/vehicle.h"
#include "entities/traffic_light.h"
#include "utils/audio_system.h"
#include "utils/resource_manager.h"
#include "world/lane.h"
#include "world/lane_road.h"
#include <algorithm>

#pragma region PROTECTED
// safe_distance = base_speed * SAFE_DISTANCE_FACTOR
float Vehicle::calcSafeDistance() const { return base_speed * Constants::SAFE_DISTANCE_FACTOR; }
#pragma endregion

#pragma region PUBLIC

#pragma region CONSTRUCTOR, DESTRUCTOR
// base_speed, safe_distance, velocity are calculated & set automactically
Vehicle::Vehicle(const Constants::DynamicObjectType& init_type,
                 const Constants::ObjectColor& init_color, Lane* init_lane)
    : DynamicObject(init_type, init_color, init_lane)
{
    setBaseSpeed(randomSpeed(init_type));     // set safe_distance automatically here
    setVelocity({base_speed * direction, 0}); // direction is already set by DynamicObject

    next_sound_time_ = randomSoundInterval();

    static std::mt19937 rng(std::random_device{}()); // khoi tao tieng xe tu khi vo game
    std::uniform_real_distribution<float> start_dist(0.0f, next_sound_time_);
    sound_timer_ = start_dist(rng);
}

float Vehicle::randomSoundInterval() const
{
    static std::mt19937 rng(std::random_device{}());
    // Random từ 2.0s đến 5.0s
    std::uniform_real_distribution<float> dist(15.0f, 35.0f);
    return dist(rng);
}

#pragma endregion

#pragma region GET, SET
float Vehicle::getBaseSpeed() const { return base_speed; }

// set base_speed & safe_distance
void Vehicle::setBaseSpeed(const float& new_speed)
{
    base_speed = new_speed;
    safe_distance = calcSafeDistance();
}

float Vehicle::getSafeDistance() const { return safe_distance; }
#pragma endregion

#pragma region OTHER
void Vehicle::playHornSound()
{
    if (getType() == Constants::DynamicObjectType::Car)
    {
        Audio().playSFX(AudioSystem::SFX::CarHorn);
    }
    else if (getType() == Constants::DynamicObjectType::Truck)
    {
        Audio().playSFX(AudioSystem::SFX::TruckHorn);
    }
}

// check if object out of map, check the nearest traffic or vehicle to reduce current speed, move
// object to new position
void Vehicle::updateState(float delta_time)
{
    if (isOutOfLane() || !is_active)
        return;

    // bóp còi mỗi 2-5s
    sound_timer_ += delta_time;
    if (sound_timer_ >= next_sound_time_)
    {
        playHornSound();
        sound_timer_ = 0.f;                       // Reset lại bộ đếm
        next_sound_time_ = randomSoundInterval(); // Random thời gian chờ cho lần kêu tiếp theo
    }

    float v_light = base_speed;
    float v_vehicle = base_speed;
    bool is_blocked = false;

    LaneRoad* road = dynamic_cast<LaneRoad*>(current_lane);
    Traffic* nearest_traffic = road ? road->getNearestTrafficAhead(*this) : nullptr;
    if (nearest_traffic)
    {
        float gap =
            gapToObstacle(nearest_traffic->getPosition().x, nearest_traffic->getWidthSprite());
        float ratio = std::clamp(gap / safe_distance, 0.f, 1.f);
        v_light = base_speed * ratio;
        if (ratio < 0.3f)
            is_blocked = true;
    }
    Vehicle* nearest_vehicle = road ? road->getNearestVehicleAhead(*this) : nullptr;
    if (nearest_vehicle)
    {
        float gap =
            gapToObstacle(nearest_vehicle->getPosition().x, nearest_vehicle->getWidthSprite());
        float ratio = std::clamp(gap / safe_distance, 0.f, 1.f);
        float nearest_vehicle_speed = nearest_vehicle->getVelocity().x * direction;
        v_vehicle = nearest_vehicle_speed + ratio * (base_speed - nearest_vehicle_speed);
        if (ratio < 0.5f)
            is_blocked = true;
    }

    // if (is_blocked && (velocity.x * direction) < (base_speed * 0.5f))
    // {
    //     // Random tỷ lệ 1% mỗi frame để bóp còi (tránh spam âm thanh gây chói tai)
    //     if ((rand() % 250) < 1)
    //     {
    //         playHornSound();
    //     }
    // }

    float v_target = std::min(std::min(base_speed, v_light), v_vehicle);
    float acceleration = (base_speed * base_speed) / (safe_distance);
    if (v_target > velocity.x * direction &&
        (velocity.x * direction + acceleration * delta_time) <= base_speed)
    {
        v_target = std::min(v_target, velocity.x * direction + acceleration * delta_time);
    }
    setVelocity({v_target * direction, 0.0f});
    moveBy(velocity * delta_time);

    anim_timer_ += delta_time;

    if (Resource().hasSpritesheet(getType()))
    {
        setTextureRect(Resource().getDynamicObjectSpritesheetRect(getType(), anim_timer_));
    }
    else
    {
        setSprite(Resource().getDynamicObjectTexture(getType(), getColor()),
                  getType());
    }
}
#pragma endregion
#pragma endregion
