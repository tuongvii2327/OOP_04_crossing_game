#include "entities/coin.h"
#include "entities/wood.h"
#include "utils/resource_manager.h"
#include "world/lane.h"
#include <cmath>

#pragma region PUBLIC

#pragma region CONSTRUCTOR, DESTRUCTOR
Coin::Coin(Lane* init_lane, const sf::Vector2f& init_position,
           const Constants::ObjectColor& init_color)
    : StaticObject(init_lane, Constants::StaticObjectType::Coin, init_position, init_color),
      coin_color(init_color), is_collected_(false)
{
    sprite.setTexture(
        Resource().getStaticObjectTexture(Constants::StaticObjectType::Coin, init_color));

    sf::Vector2f current_scale = sprite.getScale();
    sprite.setScale({std::abs(current_scale.x), std::abs(current_scale.y)});

    setPosition(init_position);
}
#pragma endregion

#pragma region GET, SET

void Coin::setPosition(const sf::Vector2f& pos)
{
    sf::Vector2f cur_scale = sprite.getScale();
    sprite.setScale({std::abs(cur_scale.x), std::abs(cur_scale.y)});
    sprite.setOrigin({0.f, 0.f});

    if (parent_wood_ != nullptr || velocity_.x != 0.f || velocity_.y != 0.f)
    {
        position = pos;
        if (current_lane != nullptr)
        {
            position.y = current_lane->getPosition().y + (Constants::LANE_HEIGHT - getHeightSprite()) / 2.f;
        }
        sprite.setPosition(position);
        hitbox.position = position;
        hitbox.size = sf::Vector2f(getWidthSprite(), getHeightSprite());
    }
    else
    {
        StaticObject::setPosition(pos);
        if (current_lane != nullptr)
        {
            float cell_x = position.x - (Constants::CELL_SIZE - getWidthSprite()) / 2.f;
            float lane_y = current_lane->getPosition().y;

            hitbox.position = sf::Vector2f(cell_x, lane_y);
            hitbox.size = sf::Vector2f(Constants::CELL_SIZE, Constants::LANE_HEIGHT);
        }
    }
}

Constants::StaticObjectType Coin::getType() const { return Constants::StaticObjectType::Coin; }
#pragma endregion

#pragma region OTHER

void Coin::updateState(float delta_time) 
{ 
    if (parent_wood_ != nullptr)
    {
        if (!parent_wood_->isActive())
        {
            return;
        }
        velocity_ = parent_wood_->getVelocity();
        // Tọa độ X luôn dính chặt vào vị trí khúc gỗ + offset_x
        position.x = parent_wood_->getPosition().x + offset_x_on_wood_;
        if (current_lane != nullptr)
        {
            position.y = current_lane->getPosition().y + (Constants::LANE_HEIGHT - getHeightSprite()) / 2.f;
        }
    }
    else if (velocity_.x != 0.f || velocity_.y != 0.f)
    {
        position += velocity_ * delta_time;
    }

    setPosition(position); 
}


void Coin::render(sf::RenderWindow& window) const
{
    if (!is_collected_)
    {
        StaticObject::render(window);
    }
}
#pragma endregion
#pragma endregion