#pragma once

#include "animal.h"

class Bird : public Animal
{
protected:
	Constants::ObjectColor bird_color;

public:
	// contructor, destructor
	Bird(const Constants::ObjectColor &init_color = Constants::ObjectColor::Pink, Lane* init_lane = nullptr);

	// get, set
	Constants::ObjectColor getColor() const override;
	void setColor(const Constants::ObjectColor &type);

	Constants::ObjectType getType() const override;
};
