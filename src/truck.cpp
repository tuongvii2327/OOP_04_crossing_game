#include "../include/entities/truck.h"
#include "../include/world/lane.h"

#pragma region PUBLIC
#pragma region CONSTRUCTOR, DESTRUCTOR
//include +setTruckType()
Truck::Truck(const sf::Texture& texture,
	Constants::TruckType type,
	Lane* init_lane)
	: Vehicle(texture, Constants::DynamicObjectType::Truck, init_lane)
{
	setTruckType(type);
}
#pragma endregion 


#pragma region GET, SET
Constants::TruckType Truck::getTruckType() const
{
	return truck_type;
}

void Truck::setTruckType(const Constants::TruckType& type)
{
	truck_type = type;
}

Constants::DynamicObjectType Truck::getType() const
{
	return Constants::DynamicObjectType::Truck;
}
#pragma endregion 
#pragma endregion 
