#include "entities/bird.h"
#include "utils/resource_manager.h"
#include "world/lane.h"

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
	if (Resource().hasSpritesheet(Constants::DynamicObjectType::Bird))
	{
		setTextureRect(Resource().getDynamicObjectSpritesheetRect(Constants::DynamicObjectType::Bird, anim_timer_, !is_move));
	}
	else
	{
		setSprite(Resource().getDynamicObjectTexture(Constants::DynamicObjectType::Bird, new_color),
			Constants::DynamicObjectType::Bird);
	}
}

Constants::DynamicObjectType Bird::getType() const
{
	return Constants::DynamicObjectType::Bird;
}

// tick value held while the bird is stopped; the spritesheet rect is resolved
// to the standing cell (r1c1) via the is_stopped flag in getDynamicObjectSpritesheetRect()
float Bird::getStoppedTimer() const
{
	return Constants::ANIMATION_TICK;
}
#pragma endregion
#pragma endregion
