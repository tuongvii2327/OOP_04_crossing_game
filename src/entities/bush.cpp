#include "entities/bush.h"
#include "world/lane.h"

#pragma region PUBLIC

#pragma region CONSTRUCTOR, DESTRUCTOR
Bush::Bush(Lane* init_lane,
    const sf::Vector2f& init_position,
    const Constants::ObjectColor& init_color)
    : StaticObject(init_lane, Constants::StaticObjectType::Bush, init_position, init_color)
{
    bush_color = init_color;
}
#pragma endregion

#pragma region GET, SET
Constants::StaticObjectType Bush::getType() const
{
    return Constants::StaticObjectType::Bush;
}
#pragma endregion

#pragma region OTHER
// keep the bush vertically locked to its lane as the map scrolls along Oy
void Bush::updateState(float delta_time)
{
    setPosition(getPosition());
}
#pragma endregion
#pragma endregion
