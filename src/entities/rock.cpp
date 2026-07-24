#include "entities/rock.h"
#include "world/lane.h"

#pragma region PUBLIC

#pragma region CONSTRUCTOR, DESTRUCTOR
Rock::Rock(Lane *init_lane, const sf::Vector2f &init_position)
    : StaticObject(Constants::StaticObjectType::Rock, init_position, init_lane, Constants::ObjectColor::None)
{
}
#pragma endregion

#pragma region GET, SET
Constants::StaticObjectType Rock::getType() const
{
    return Constants::StaticObjectType::Rock;
}
#pragma endregion

#pragma region OTHER
// keep the rock vertically locked to its lane as the map scrolls along Oy
void Rock::updateState(float delta_time)
{
    setPosition(getPosition());
}
#pragma endregion
#pragma endregion
