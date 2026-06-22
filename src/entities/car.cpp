#include "../include/entities/car.h"
#include "../include/world/lane.h"

#pragma region PUBLIC
#pragma region CONSTRUCTOR, DESTRUCTOR
// 
Car::Car(const Constants::ObjectColor& init_color,
	Lane* init_lane)
	: Vehicle(Constants::ObjectType::Car, init_color, init_lane)
{
	car_color = init_color;
}
#pragma endregion 


#pragma region GET, SET
Constants::ObjectColor Car::getColor() const
{
	return car_color;
}

void Car::setColor(const Constants::ObjectColor& color)
{
	car_color = color;
	setSprite(getTexture(Constants::ObjectType::Car, color));
}

Constants::ObjectType Car::getType() const
{
	return Constants::ObjectType::Car;
}
#pragma endregion 
#pragma endregion 
