#include "../include/entities/rabit.h"
#include "../include/world/lane.h"

#pragma region PUBLIC
#pragma region CONSTRUCTOR, DESTRUCTOR
Rabit::Rabit(const sf::Texture& texture,
	//constants color
	Lane* init_lane)
	: Animal(texture, Constants::DynamicObjectType::Rabit, init_lane)
{
	//color
}
#pragma endregion
#pragma region GET, SET
Constants::DynamicObjectType Rabit::getType() const
{
	return Constants::DynamicObjectType::Rabit;
}
#pragma endregion
#pragma endregion
