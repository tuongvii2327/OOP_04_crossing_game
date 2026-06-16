#include "../include/entities/car.h"

#pragma region PUBLIC
#pragma region CONSTRUCTOR, DESTRUCTOR
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

//returns DynamicObjectType::Car
Constants::DynamicObjectType Car::getType() const
{
	return Constants::DynamicObjectType::Car;
}
#pragma endregion 


#pragma region SAVE/LOAD

////write state of object to binary file
//void Car::serialize(std::ostream& outDev) const
//{
//	Vehicle::serialize(outDev); //writes position, velocity, is_active, direction, base_speed
//	outDev.write(reinterpret_cast<const char*>(&car_type), sizeof(car_type));
//}

////read state of object from binary file
//void Car::deserialize(std::istream& inDev)
//{
//	Vehicle::deserialize(inDev); //reads base_speed and rebuilds the live state
//	inDev.read(reinterpret_cast<char*>(&car_type), sizeof(car_type));
//}
#pragma endregion 
#pragma endregion 
