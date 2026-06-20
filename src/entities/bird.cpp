#include "../include/entities/bird.h"
#include "../include/world/lane.h"

#pragma region PUBLIC
#pragma region CONSTRUCTOR, DESTRUCTOR
Bird::Bird(const sf::Texture& texture,
	//constants color
	Lane* init_lane)
	: Animal(texture, Constants::DynamicObjectType::Bird, init_lane)
{
	//color
}
#pragma endregion
#pragma region GET, SET
Constants::DynamicObjectType Bird::getType() const
{
	return Constants::DynamicObjectType::Bird;
}
#pragma endregion
#pragma endregion
