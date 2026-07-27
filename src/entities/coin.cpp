#include "entities/coin.h"
#include "utils/resource_manager.h"
#include <cmath>

Coin::Coin(const sf::Vector2f &cell_top_left, Lane *init_lane)
    : StaticObject(init_lane, Constants::StaticObjectType::Coin, cell_top_left, Constants::ObjectColor::Yellow),
      is_collected_(false)
{
    // Nạp Texture ảnh
    sprite.setTexture(Resource().getStaticObjectTexture(Constants::StaticObjectType::Coin, Constants::ObjectColor::Yellow));

    sprite.setOrigin({0.f, 0.f});

    // Khử dấu âm Scale - tránh lật ngược đồng xu
    sf::Vector2f current_scale = sprite.getScale();
    sprite.setScale({std::abs(current_scale.x), std::abs(current_scale.y)});

    // Căn giữa tự động
    sf::FloatRect bounds = sprite.getGlobalBounds();
    float img_w = bounds.size.x;
    float img_h = bounds.size.y;


    float offset_x = (Constants::CELL_SIZE - img_w) / 2.0f;
    float offset_y = (Constants::LANE_HEIGHT - img_h) / 2.0f;

    sf::Vector2f centered_pos = {cell_top_left.x + offset_x, cell_top_left.y + offset_y};
    setPosition(centered_pos);

    hitbox.size = sf::Vector2f(Constants::CELL_SIZE, Constants::LANE_HEIGHT);
    hitbox_offset = sf::Vector2f(-offset_x, -offset_y); // Lùi Hitbox ra sát mép Cell
    hitbox.position = centered_pos + hitbox_offset;      // Hitbox bắt đầu đúng tại cell_top_left
}