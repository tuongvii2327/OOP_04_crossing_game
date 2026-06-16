#include "../include/entities/truck.h"

#pragma region PUBLIC
#pragma region CONSTRUCTOR, DESTRUCTOR
Truck::Truck(const sf::Texture& texture,
	Constants::TruckType type,
	Lane* init_lane)
	: Vehicle(texture, Constants::DynamicObjectType::Truck, init_lane)
{
	this->setTruckType(type);
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

//returns DynamicObjectType::Truck
Constants::DynamicObjectType Truck::getType() const
{
	return Constants::DynamicObjectType::Truck;
}
#pragma endregion 


#pragma region SAVE/LOAD

////write state of object to binary file
//void Truck::serialize(std::ostream& outDev) const
//{
//	Vehicle::serialize(outDev); //writes position, velocity, is_active, direction, base_speed
//	outDev.write(reinterpret_cast<const char*>(&truck_type), sizeof(truck_type));
//}

////read state of object from binary file
//void Truck::deserialize(std::istream& inDev)
//{
//	Vehicle::deserialize(inDev); //reads base_speed and rebuilds the live state
//	inDev.read(reinterpret_cast<char*>(&truck_type), sizeof(truck_type));
//}
#pragma endregion 
#pragma endregion 
