#include "entities/coin.h"
#include "world/lane.h"
#include "utils/resource_manager.h"
#include <cmath>

#pragma region PUBLIC

#pragma region CONSTRUCTOR, DESTRUCTOR
Coin::Coin(Lane* init_lane,
		   const sf::Vector2f &init_position,
		   const Constants::ObjectColor &init_color)
    : StaticObject(init_lane, Constants::StaticObjectType::Coin, init_position, init_color),
      coin_color(init_color),
      is_collected_(false)
{
	// Nạp Texture từ ResourceManager
	sprite.setTexture(Resource().getStaticObjectTexture(Constants::StaticObjectType::Coin, init_color));

	// Khử dấu âm Scale - tránh lật ngược đồng xu khi lane chạy sang trái
	sf::Vector2f current_scale = sprite.getScale();
	sprite.setScale({std::abs(current_scale.x), std::abs(current_scale.y)});

	setPosition(init_position);

	// Cài đặt Hitbox chuẩn bằng đúng ô Cell để dễ nhặt xu
	hitbox.size = sf::Vector2f(Constants::CELL_SIZE, Constants::LANE_HEIGHT);
	hitbox_offset = sf::Vector2f(0.f, 0.f);
	hitbox.position = init_position;
}
#pragma endregion

#pragma region GET, SET
Constants::StaticObjectType Coin::getType() const
{
	return Constants::StaticObjectType::Coin;
}
#pragma endregion

#pragma region OTHER

void Coin::updateState(float delta_time)
{
	setPosition(getPosition());
}

void Coin::render(sf::RenderWindow &window) const
{
	if (!is_collected_)
	{
		StaticObject::render(window);
	}
}
#pragma endregion
#pragma endregion