#pragma once

#include "animal.h"

class Bird : public Animal
{
private:
	Constants::ObjectColor bird_color;

public:
	// contructor, destructor
	Bird(const Constants::ObjectColor &init_color = Constants::ObjectColor::Green, 
		Lane* init_lane = nullptr);

	// get, set
	Constants::ObjectColor getColor() const override;
	void setColor(const Constants::ObjectColor &new_color);

	Constants::DynamicObjectType getType() const override;
};
