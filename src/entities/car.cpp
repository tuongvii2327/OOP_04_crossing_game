#include "entities/car.h"
#include "world/lane.h"
#include "utils/resource_manager.h"

#pragma region PUBLIC
#pragma region CONSTRUCTOR, DESTRUCTOR
// fixed type Car, base_speed/safe_distance/velocity set by Vehicle
Car::Car(const Constants::ObjectColor &init_color, 
    Lane *init_lane)
    : Vehicle(Constants::DynamicObjectType::Car, init_color, init_lane)
{
    car_color = init_color;
}
#pragma endregion

#pragma region GET, SET
Constants::ObjectColor Car::getColor() const
{
    return car_color;
}

void Car::setColor(const Constants::ObjectColor &new_color)
{
    car_color = new_color;
    if (Resource().hasSpritesheet(Constants::DynamicObjectType::Car))
    {
        setTextureRect(Resource().getDynamicObjectSpritesheetRect(Constants::DynamicObjectType::Car, current_frame));
    }
    else
    {
        setSprite(Resource().getDynamicObjectTexture(Constants::DynamicObjectType::Car, new_color, current_frame), Constants::DynamicObjectType::Car);
    }
}

Constants::DynamicObjectType Car::getType() const
{
    return Constants::DynamicObjectType::Car;
}
#pragma endregion
#pragma endregion
