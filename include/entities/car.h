#pragma once

#include "vehicle.h"

class Car : public Vehicle
{
private:
	Constants::ObjectColor car_color;

public:
	// contructor, destructor
	Car(const Constants::ObjectColor &init_color = Constants::ObjectColor::Pink, Lane* init_lane = nullptr);

	// get, set
	Constants::ObjectColor getColor() const override;
	void setColor(const Constants::ObjectColor &type);

	Constants::ObjectType getType() const override;
};
