#include "../include/entities/bird.h"
#include "../include/world/lane.h"

#pragma region PUBLIC
#pragma region CONSTRUCTOR, DESTRUCTOR
Bird::Bird(const Constants::ObjectColor &init_color, Lane* init_lane)
	: Animal(Constants::ObjectType::Bird, init_color, init_lane)
{
	bird_color = init_color;
}
#pragma endregion

#pragma region GET, SET

Constants::ObjectColor Bird::getColor() const
{
	return bird_color;
}

// include #getTexture(), +setSprite()
void Bird::setColor(const Constants::ObjectColor &color)
{
	bird_color = color;
	setSprite(getTexture(Constants::ObjectType::Bird, color));
}

Constants::ObjectType Bird::getType() const
{
	return Constants::ObjectType::Bird;
}
#pragma endregion
#pragma endregion
