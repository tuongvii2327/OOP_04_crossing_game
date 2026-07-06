#pragma once

#include "animal.h"

class Rabbit : public Animal
{
private:
	Constants::ObjectColor rabbit_color;

public:
	// contructor, destructor
	Rabbit(const Constants::ObjectColor &init_color = Constants::ObjectColor::Pink, Lane* init_lane = nullptr);

	// get, set
	Constants::ObjectColor getColor() const override;
	void setColor(const Constants::ObjectColor &new_color);

	Constants::DynamicObjectType getType() const override;
};
