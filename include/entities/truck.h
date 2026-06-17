#pragma once

#include "vehicle.h"

class Truck : public Vehicle
{
private:
	Constants::TruckType truck_type;

public:
	//contructor, destructor
	Truck(const sf::Texture& texture,
		Constants::TruckType type = Constants::TruckType::RedTruck,
		Lane* init_lane = nullptr);

	//get, set
	Constants::TruckType getTruckType() const;
	void setTruckType(const Constants::TruckType& type);

	Constants::DynamicObjectType getType() const override;
};
