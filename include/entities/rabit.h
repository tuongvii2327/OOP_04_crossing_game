#pragma once

#include "animal.h"

class Rabit : public Animal
{
protected:
	Constants::ObjectColor rabit_color;

public:
	// contructor, destructor
	Rabit(const Constants::ObjectColor &init_color = Constants::ObjectColor::Pink, Lane* init_lane = nullptr);

	// get, set
	Constants::ObjectColor getColor() const override;
	void setColor(const Constants::ObjectColor &type);

	Constants::ObjectType getType() const override;
};
