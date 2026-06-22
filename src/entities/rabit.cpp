#include "../include/entities/rabit.h"
#include "../include/world/lane.h"

#pragma region PUBLIC
#pragma region CONSTRUCTOR, DESTRUCTOR
Rabit::Rabit(const Constants::ObjectColor& init_color,
	Lane* init_lane)
	: Animal(Constants::ObjectType::Rabit, init_color, init_lane)
{
	rabit_color = init_color;
}
#pragma endregion

#pragma region GET, SET

Constants::ObjectColor Rabit::getColor() const
{
	return rabit_color;
}

void Rabit::setColor(const Constants::ObjectColor& color)
{
	rabit_color = color;
	setSprite(getTexture(Constants::ObjectType::Rabit, color));
}

Constants::ObjectType Rabit::getType() const
{
	return Constants::ObjectType::Rabit;
}
#pragma endregion
#pragma endregion