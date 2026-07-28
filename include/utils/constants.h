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
    const int VISUAL_DISTANCE = 15;

    // hitbox size as a fraction of the sprite size (applies to every dynamic object)
    constexpr float HITBOX_SIZE_RATIO = 0.95f;

#pragma region LANE
    const unsigned int LANE_HEIGHT = 105;
    const unsigned int LANE_WIDTH = 1260;
    const unsigned int MAX_OBJECT_PER_LANE = 4;

	enum class LaneType
	{
		Grass,
		Road,
		Safe,
		River
	};

	inline int getNumberOfTextureVariant(const LaneType &type)
	{
		switch (type)
		{
		case LaneType::Grass:
			return 21;
		case LaneType::Safe:
			return 21;
		case LaneType::Road:
			return 3;
		case LaneType::River:
			return 1;
		default:
			return 1;
		}
	}
#pragma endregion

#pragma region CAMERA
	constexpr float CAMERA_SAFE_DISTANCE = 3.f * LANE_HEIGHT;
	constexpr float CAMERA_FOLLOW_LANES_PER_SEC = 3.f;
	constexpr float AUTO_SCROLL_BASE_SPEED = 60.f;
	constexpr float AUTO_SCROLL_LEVEL_FACTOR = 0.3f;
#pragma endregion

#pragma region OBJECT
    // coefficients in the 'speed' formula above
    constexpr float SPEED_LEVEL_FACTOR = 0.2f;

    enum class DynamicObjectType
    {
        // Character object
        People,
        // Road object
        Car,
        Truck,

        // Grass objecct
        Bird,
        Rabbit,

		// River object
		Wood
	};

    enum class StaticObjectType
    {
        // Character object

        // Road object
        Traffic,

		// Grass objecct
		Rock,
		Bush,

		// River object

		// Coin
		Coin
	};

	enum class ObjectColor
	{
		Red,
		Blue,
		Green,
		Grey,
		None // ke ep last: used as the color count in random pickers
	};

    // effect of an object (dynamic or static) on People when colliding with it
    enum class ObjectEffect
    {
        None,       // can be touched, nothing happens
        Blocked,    // blocks movement, cannot walk into it
        Lethal,     // can be touched, touching it kills the player
        Collectible // can be touched, touching it accumulates a special score (e.g. coin)
    };

    // the usable speed range of an object
    struct SpeedRange
    {
        int min_speed;
        int max_speed;
    };

    // Base speed range (px/s) for each object type before the level multiplier is applied
    // speed = random int in [min_speed, max_speed] * (1 + (level - 1) * SPEED_LEVEL_FACTOR)
    inline SpeedRange getSpeedRange(const DynamicObjectType &type)
    {
        switch (type)
        {
        case DynamicObjectType::Car:
            return {50, 80};
        case DynamicObjectType::Truck:
            return {35, 65};
        case DynamicObjectType::Bird:
            return {20, 60};
        case DynamicObjectType::Rabbit:
            return {35, 70};
		case DynamicObjectType::Wood:
			return {35, 80};
        default:
            return {0, 0};
        }
    }

    struct Size
    {
        int width;
        int height;
    };

	inline Size getSizeInGame(const StaticObjectType& type)
	{
		switch (type)
		{
		case StaticObjectType::Traffic:
			return { 50, 50 };
		case StaticObjectType::Rock:
			return { 80, 55 };
		case StaticObjectType::Bush:
			return { 90, 69 };
		case StaticObjectType::Coin:
			return { 65, 60 };
		default:
			return { 90, 90 };
		}
	}

    inline Size getSizeInGame(const DynamicObjectType &type)
    {
        switch (type)
        {
        case DynamicObjectType::People:
            return {52, 62};
        case DynamicObjectType::Car:
            return {90, 51};
        case DynamicObjectType::Truck:
            return {87, 81};
        case DynamicObjectType::Bird:
            return {53, 47};
        case DynamicObjectType::Rabbit:
            return {60, 60};
		case DynamicObjectType::Wood:
			return {210, 60};
        default:
            return {80, 200};
        }
    }

    inline int getNumberOfFrames(const DynamicObjectType &type)
    {
        switch (type)
        {
        case DynamicObjectType::People:
            return 1;
        case DynamicObjectType::Car:
            return 4;
        case DynamicObjectType::Truck:
            return 4;
        case DynamicObjectType::Bird:
            return 3;
        case DynamicObjectType::Rabbit:
            return 2;
        default:
            return 1;
        }
    }
#pragma region VEHICLE
    // factor to calculate safe_distance (safe_distance = base_speed * SAFE_DISTANCE_FACTOR)
    const unsigned int SAFE_DISTANCE_FACTOR = 2;
#pragma endregion

#pragma region ANIMAL
    const unsigned int MIN_DURATION_ANIMAL = 3;
    const unsigned int MAX_DURATION_ANIMAL = 8;
#pragma endregion

#pragma region TRAFFIC
    const float YELLOW_DURATION = 4; // second
    const unsigned int MIN_DURATION_TRAFFIC = 6;
    const unsigned int MAX_DURATION_TRAFFIC = 8;

    enum class TrafficLightColor
    {
        Red,
        Green,
        Yellow
    };
#pragma endregion

#pragma endregion
} // namespace Constants
