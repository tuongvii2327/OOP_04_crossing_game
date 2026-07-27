#include "entities/coin.h"
#include "utils/resource_manager.h"
#include <cmath>

Coin::Coin(const sf::Vector2f &position, Lane *init_lane)
    : StaticObject(init_lane, Constants::StaticObjectType::Coin, position),
      is_collected_(false)
{
    sprite.setOrigin({0.f, 0.f});
    // Khử dấu âm Scale - tránh lật ngược đồng xu
    sf::Vector2f current_scale = sprite.getScale();
    sprite.setScale({std::abs(current_scale.x), std::abs(current_scale.y)});

    setPosition(position);
}