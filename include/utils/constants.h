#pragma once

namespace Constants
{
    const unsigned int WINDOW_WIDTH = 1400;
    const unsigned int WINDOW_HEIGHT = 1050;
    const char *const GAME_TITLE = "Crossy Road";
    const unsigned int FPS_LIMIT = 24;

    // number of cell
    const unsigned int NUMBER_OF_LANES = 10;
    const unsigned int NUMBER_OF_CELLS = 12;

    const unsigned int GAME_PLAY_BAR_HEIGHT = 1050;
    const unsigned int GAME_PLAY_BAR_WIDTH = 140;
    const unsigned int CELL_SIZE = 105;

    // min distance between sprites of objects to avoid collision in visual (UI)
    const unsigned int VISUAL_DISTANCE = 15;

    // hitbox size as a fraction of the sprite size (applies to every dynamic object)
    constexpr float HITBOX_SIZE_RATIO = 0.9f;

#pragma region TRAFFIC
    const float YELLOW_DURATION = 4; // second
    const unsigned int TRAFFIC_HEIGHT = 30;
    const unsigned int TRAFFIC_WIDTH = 30;

    enum class TrafficLightColor
    {
        Red,
        Green,
        Yellow
    };

    // the usable duration of a traffic
    struct DurationRange
    {
        int min_duration;
        int max_duration;
    };

    // duration range (s) for each traffic color state
    inline DurationRange getDurationRange(TrafficLightColor color)
    {
        switch (color)
        {
        case TrafficLightColor::Red:
            return {6, 8};
        case TrafficLightColor::Green:
            return {6, 8};
        default:
            return {6, 8};
        }
    }
#pragma endregion

#pragma region LANE
    const unsigned int LANE_HEIGHT = 105;
    const unsigned int LANE_WIDTH = 1260;
#pragma endregion

#pragma region DYNAMICOBJECT
    // coefficients in the 'speed' formula above
    constexpr float SPEED_LEVEL_FACTOR = 0.2f;

    enum class DynamicObjectType
    {
        Car,
        Truck,
        Bird,
        Rabit
    };

    // the usable speed range of an object
    struct SpeedRange
    {
        int min_speed;
        int max_speed;
    };

    // Base speed range (px/s) for each object type before the level multiplier is applied
    // speed = random int in [min_speed, max_speed] * (1 + (level - 1) * SPEED_LEVEL_FACTOR)
    inline SpeedRange getSpeedRange(DynamicObjectType type)
    {
        switch (type)
        {
        case DynamicObjectType::Car:
            return {50, 80};
        case DynamicObjectType::Truck:
            return {35, 65};
        case DynamicObjectType::Bird:
            return {20, 60};
        case DynamicObjectType::Rabit:
            return {35, 70};
        default:
            return {20, 30};
        }
    }

#pragma region VEHICLE
    // factor to calculate safe_distance (safe_distance = base_speed * SAFETY_FACTOR)
    const unsigned int SAFE_DISTANCE_FACTOR = 2;

#pragma region CAR
    enum class CarType
    {
        RedCar,
        BlueCar,
        YellowCar
    };
#pragma endregion
#pragma region TRUCK
    enum class TruckType
    {
        RedTruck,
        BlueTruck,
        YellowTruck
    };
#pragma endregion
#pragma endregion
#pragma region ANIMAL
    const unsigned int MIN_DURATION_ANIMAL = 3;
    const unsigned int MAX_DURATION_ANIMAL = 8;
#pragma endregion
#pragma endregion
} // namespace Constants
