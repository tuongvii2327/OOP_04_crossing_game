#include "../include/entities/truck.h"
#include "../include/world/lane.h"

#pragma region PUBLIC
#pragma region CONSTRUCTOR, DESTRUCTOR
//
Truck::Truck(const Constants::ObjectColor &init_color, Lane* init_lane)
	: Vehicle(Constants::ObjectType::Truck, init_color, init_lane)
{
	truck_color = init_color;
}
#pragma endregion


#pragma region GET, SET
Constants::ObjectColor Truck::getColor() const
{
	return truck_color;
}

// include #getTexture(), +setSprite()
void Truck::setColor(const Constants::ObjectColor &color)
{
	truck_color = color;
	setSprite(getTexture(Constants::ObjectType::Truck, color));
}

Constants::ObjectType Truck::getType() const
{
	return Constants::ObjectType::Truck;
}
#pragma endregion
#pragma endregion
