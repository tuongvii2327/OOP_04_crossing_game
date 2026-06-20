#pragma once

#include "animal.h"

class Rabit : public Animal
{
protected:
	//add constants color
public:
	//contructor, destructor
	Rabit(const sf::Texture& texture,
		//constants color
		Lane* init_lane = nullptr);

	//get, set
	//get set constants color

	Constants::DynamicObjectType getType() const override;
};