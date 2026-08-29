#pragma once
#include <string>
namespace Constants
{
	const unsigned int WINDOW_WIDTH = 1400;
	const unsigned int WINDOW_HEIGHT = 1050;
	const char* const GAME_TITLE = "Crossy Road";
	const unsigned int FPS_LIMIT = 60;

	// Số lượng ô grid trên bản đồ
	const unsigned int NUMBER_OF_LANES = 10;
	const unsigned int NUMBER_OF_CELLS = 12;

	const unsigned int GAME_PLAY_BAR_HEIGHT = 1050;
	const unsigned int GAME_PLAY_BAR_WIDTH = 140;
	const unsigned int CELL_SIZE = 105;

	// Khoảng cách tối thiểu giữa các sprite của object để tránh va chạm trực quan (trên UI)
	const int VISUAL_DISTANCE = 15;

	// Tỷ lệ hitbox so với kích thước thật của sprite (áp dụng cho mọi đối tượng có thể di chuyển)
	constexpr float HITBOX_SIZE_RATIO = 0.95f;

	// Thời lượng của 1 khung hình hoạt ảnh (tính bằng giây).
	// 0.166s tương đương tốc độ 6 hình/giây.
	constexpr float ANIMATION_TICK = 0.166f;

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

	enum class DeathCause
	{
		None,
		Car,
		Drowning,
		OutOfBounds
	};

	inline int getNumberOfTextureVariant(const LaneType &type)
	{
		switch (type)
		{
		case LaneType::Grass:
			return 50;
		case LaneType::Safe:
			return 42;
		case LaneType::Road:
			return 3;
		case LaneType::River:
			return 7;
		default:
			return 1;
		}
	}
#pragma endregion

#pragma region CAMERA
	constexpr float CAMERA_SAFE_DISTANCE = 3.f * LANE_HEIGHT;
	constexpr float CAMERA_FOLLOW_LANES_PER_SEC = 2.5f;
	constexpr float AUTO_SCROLL_BASE_SPEED = 50.f;
	constexpr float AUTO_SCROLL_LEVEL_FACTOR = 0.25f;
#pragma endregion

#pragma region OBJECT
	// Hệ số nhân tốc độ theo level (speed_multiplier)
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
		None // keep last: used as the color count in random pickers
	};

	// Hiệu ứng của một đối tượng (động hoặc tĩnh) tác động lên nhân vật (People) khi xảy ra va chạm
	enum class ObjectEffect
	{
		None,       // Có thể đi xuyên qua, không có chuyện gì xảy ra
		Blocked,    // Chặn đường đi, nhân vật không thể đi qua
		Lethal,     // Gây chết người nếu nhân vật chạm vào
		Collectible // Có thể thu thập để cộng điểm (vd: Coin)
	};

	// Khoảng tốc độ có thể sử dụng của một đối tượng
	struct SpeedRange
	{
		int min_speed;
		int max_speed;
	};

	// Lấy khoảng tốc độ cơ bản (px/s) cho mỗi loại đối tượng trước khi áp dụng hệ số nhân của level.
	// Công thức: speed = random int trong [min_speed, max_speed] * (1 + (level - 1) * SPEED_LEVEL_FACTOR)
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

	inline Size getSizeInGame(const StaticObjectType &type)
	{
		switch (type)
		{
		case StaticObjectType::Traffic:
			return {105, 105};
		case StaticObjectType::Rock:
			return {102, 70};
		case StaticObjectType::Bush:
			return {90, 69};
		case StaticObjectType::Coin:
			return {65, 60};
		default:
			return {90, 90};
		}
	}

	inline Size getSizeInGame(const DynamicObjectType &type)
	{
		switch (type)
		{
		case DynamicObjectType::People:
			return {75, 78};
		case DynamicObjectType::Car:
			return {90, 51};
		case DynamicObjectType::Truck:
			return {105, 100};
		case DynamicObjectType::Bird:
			return {73, 60};
		case DynamicObjectType::Rabbit:
			return {70, 70};
		case DynamicObjectType::Wood:
			return {210, 100};
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
	// Hệ số dùng để tính toán khoảng cách an toàn (safe_distance = base_speed * SAFE_DISTANCE_FACTOR)
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
	// Khoảng cách nâng đèn giao thông lên phía trên so với mép trên của lane
	const float TRAFFIC_LIGHT_Y_OFFSET = 40.f;

	enum class TrafficLightColor
	{
		Red,
		Green,
		Yellow
	};
#pragma endregion

#pragma region SPRITESHEET
	constexpr int SPRITESHEET_FRAME_SIZE = 800;

	struct SpriteFrameRect
	{
		int content_width;
		int content_height;
		int offset_x;
		int offset_y;
	};

	inline SpriteFrameRect getSpriteFrameRect(const DynamicObjectType &type)
	{
		switch (type)
		{
		case DynamicObjectType::Car:
			return {750, 425, 25, 250};
		case DynamicObjectType::Rabbit:
			return {700, 700, (SPRITESHEET_FRAME_SIZE - 700) / 2, (SPRITESHEET_FRAME_SIZE - 700) / 2};
		case DynamicObjectType::Bird:
			return {725, 643, (SPRITESHEET_FRAME_SIZE - 725) / 2, (SPRITESHEET_FRAME_SIZE - 643) / 2};
		case DynamicObjectType::Truck:
			return {725, 675, (SPRITESHEET_FRAME_SIZE - 725) / 2, (SPRITESHEET_FRAME_SIZE - 675) / 2};
		default:
			return {SPRITESHEET_FRAME_SIZE, SPRITESHEET_FRAME_SIZE, 0, 0};
		}
	}
#pragma endregion

	inline std::string getObjectColorString(const ObjectColor &color)
	{
		switch (color)
		{
		case ObjectColor::Red:
			return "red";
		case ObjectColor::Blue:
			return "blue";
		case ObjectColor::Green:
			return "green";
		case ObjectColor::Grey:
			return "grey";
		case ObjectColor::None:
			return "none";
		default:
			return "none";
		}
	}

} // namespace Constants
