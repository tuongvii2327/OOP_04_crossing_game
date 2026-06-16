#pragma once

#include "vehicle.h"

class Car : public Vehicle
{
private:
	Constants::CarType car_type;

public:
	//contructor, destructor
	Car(const sf::Texture& texture,
		Constants::CarType type = Constants::CarType::RedCar,
		Lane* init_lane = nullptr);

	//get, set
	Constants::CarType getCarType() const;
	void setCarType(const Constants::CarType& type);

	Constants::DynamicObjectType getType() const override; //returns DynamicObjectType::Car

	//Save/Load
	//void serialize(std::ostream& outDev) const override;  //write state of object to binary file
	//void deserialize(std::istream& inDev) override; //read state of object from binary file
};
