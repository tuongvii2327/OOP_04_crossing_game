#pragma once

#include "animal.h"

class Bird : public Animal
{
protected:
	//add constants color
public:
	//contructor, destructor
	Bird(const sf::Texture& texture,
		//constants color
		Lane* init_lane = nullptr);

	//get, set
	//get set constants color

	Constants::DynamicObjectType getType() const override;
};