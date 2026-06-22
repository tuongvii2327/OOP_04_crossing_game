#include "../include/entities/car.h"
#include "../include/world/lane.h"

#pragma region PUBLIC
#pragma region CONSTRUCTOR, DESTRUCTOR
//include +setCarType()
Car::Car(const sf::Texture& texture,
	Constants::CarType type,
	Lane* init_lane)
	: Vehicle(texture, Constants::DynamicObjectType::Car, init_lane)
{
	setCarType(type);
}
#pragma endregion 


#pragma region GET, SET
Constants::CarType Car::getCarType() const
{
	return car_type;
}

void Car::setCarType(const Constants::CarType& type)
{
	car_type = type;
}

Constants::DynamicObjectType Car::getType() const
{
	return Constants::DynamicObjectType::Car;
}
#pragma endregion 
#pragma endregion 
