#include "../include/entities/bird.h"
#include "../include/world/lane.h"

#pragma region PUBLIC
#pragma region CONSTRUCTOR, DESTRUCTOR
Bird::Bird(const Constants::ObjectColor &init_color, Lane* init_lane)
	: Animal(Constants::DynamicObjectType::Bird, init_color, init_lane)
{
	bird_color = init_color;
}
#pragma endregion

#pragma region GET, SET

Constants::ObjectColor Bird::getColor() const
{
	return bird_color;
}

void Bird::setColor(const Constants::ObjectColor &new_color)
{
	bird_color = new_color;
	setSprite(getTexture(Constants::DynamicObjectType::Bird, new_color));
}

Constants::DynamicObjectType Bird::getType() const
{
	return Constants::DynamicObjectType::Bird;
}
#pragma endregion
#pragma endregion
