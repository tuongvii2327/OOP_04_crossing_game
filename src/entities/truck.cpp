#include "entities/truck.h"
#include "world/lane.h"
#include "utils/resource_manager.h"

#pragma region PUBLIC
#pragma region CONSTRUCTOR, DESTRUCTOR
// fixed type Truck, base_speed/safe_distance/velocity set by Vehicle
Truck::Truck(const Constants::ObjectColor &init_color, 
    Lane *init_lane)
    : Vehicle(Constants::DynamicObjectType::Truck, init_color, init_lane)
{
    truck_color = init_color;
}
#pragma endregion

#pragma region GET, SET
Constants::ObjectColor Truck::getColor() const
{
    return truck_color;
}

void Truck::setColor(const Constants::ObjectColor &new_color)
{
    truck_color = new_color;
    setSprite(Resource().getDynamicObjectTexture(Constants::DynamicObjectType::Truck, new_color, current_frame), Constants::DynamicObjectType::Truck);
}

Constants::DynamicObjectType Truck::getType() const
{
    return Constants::DynamicObjectType::Truck;
}
#pragma endregion
#pragma endregion
