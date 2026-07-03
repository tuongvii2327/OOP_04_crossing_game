#pragma once

#include "vehicle.h"

class Truck : public Vehicle
{
private:
	Constants::ObjectColor truck_color;

public:
	// contructor, destructor
	Truck(const Constants::ObjectColor &init_color = Constants::ObjectColor::Pink, Lane* init_lane = nullptr);

	// get, set
	Constants::ObjectColor getColor() const override;
	void setColor(const Constants::ObjectColor &color);

	Constants::ObjectType getType() const override;
};
