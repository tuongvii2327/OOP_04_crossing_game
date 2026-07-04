#include "../include/entities/rabbit.h"
#include "../include/world/lane.h"

#pragma region PUBLIC
#pragma region CONSTRUCTOR, DESTRUCTOR
Rabbit::Rabbit(const Constants::ObjectColor &init_color, Lane* init_lane)
	: Animal(Constants::ObjectType::Rabbit, init_color, init_lane)
{
	rabbit_color = init_color;
}
#pragma endregion

#pragma region GET, SET

Constants::ObjectColor Rabbit::getColor() const
{
	return rabbit_color;
}

// include #getTexture(), +setSprite()
void Rabbit::setColor(const Constants::ObjectColor &color)
{
	rabbit_color = color;
	setSprite(getTexture(Constants::ObjectType::Rabbit, color));
}

Constants::ObjectType Rabbit::getType() const
{
	return Constants::ObjectType::Rabbit;
}
#pragma endregion
#pragma endregion
