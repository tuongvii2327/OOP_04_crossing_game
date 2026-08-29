#include "entities/rabbit.h"
#include "utils/resource_manager.h"
#include "world/lane.h"

#pragma region PUBLIC
#pragma region CONSTRUCTOR, DESTRUCTOR
Rabbit::Rabbit(const Constants::ObjectColor& init_color, Lane* init_lane)
    : Animal(Constants::DynamicObjectType::Rabbit, init_color, init_lane)
{
    rabbit_color = init_color;
}
#pragma endregion

#pragma region GET, SET

Constants::ObjectColor Rabbit::getColor() const { return rabbit_color; }

void Rabbit::setColor(const Constants::ObjectColor& new_color)
{
    rabbit_color = new_color;
    if (Resource().hasSpritesheet(Constants::DynamicObjectType::Rabbit))
    {
        setTextureRect(Resource().getDynamicObjectSpritesheetRect(
            Constants::DynamicObjectType::Rabbit, anim_timer_));
    }
    else
    {
        setSprite(Resource().getDynamicObjectTexture(Constants::DynamicObjectType::Rabbit,
                                                     new_color),
                  Constants::DynamicObjectType::Rabbit);
    }
}

Constants::DynamicObjectType Rabbit::getType() const
{
    return Constants::DynamicObjectType::Rabbit;
}
#pragma endregion
#pragma endregion
