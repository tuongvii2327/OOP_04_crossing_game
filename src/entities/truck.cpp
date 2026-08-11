#include "entities/truck.h"
#include "utils/resource_manager.h"
#include "world/lane.h"

#pragma region PUBLIC
#pragma region CONSTRUCTOR, DESTRUCTOR
// fixed type Truck, base_speed/safe_distance/velocity set by Vehicle
Truck::Truck(const Constants::ObjectColor& init_color, Lane* init_lane)
    : Vehicle(Constants::DynamicObjectType::Truck, init_color, init_lane)
{
    truck_color = init_color;
}
#pragma endregion

#pragma region GET, SET
Constants::ObjectColor Truck::getColor() const { return truck_color; }

void Truck::setColor(const Constants::ObjectColor& new_color)
{
    truck_color = new_color;
    const sf::Texture& tex =
        Resource().getDynamicObjectSpritesheet(Constants::DynamicObjectType::Truck, new_color);
    sprite.setTexture(tex, true);
    setTextureRect(Resource().getDynamicObjectSpritesheetRect(Constants::DynamicObjectType::Truck,
                                                              current_frame));
    adjustSprite(Constants::DynamicObjectType::Truck);
    applyFacing();
    setHitbox();
}

Constants::DynamicObjectType Truck::getType() const { return Constants::DynamicObjectType::Truck; }
#pragma endregion
#pragma endregion
