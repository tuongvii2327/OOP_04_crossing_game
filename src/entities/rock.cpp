#include "entities/rock.h"
#include "world/lane.h"

#pragma region PUBLIC

#pragma region CONSTRUCTOR, DESTRUCTOR
Rock::Rock(Lane* init_lane, const sf::Vector2f& init_position,
           const Constants::ObjectColor& init_color)
    : StaticObject(init_lane, Constants::StaticObjectType::Rock, init_position, init_color)
{
    rock_color = init_color;
    setPosition(init_position);
    if (current_lane != nullptr)
    {
        float cell_x = position.x - (Constants::CELL_SIZE - getWidthSprite()) / 2.f;
        float lane_y = current_lane->getPosition().y;

        hitbox.position = sf::Vector2f(cell_x + 1.f, lane_y + 1.f);
        hitbox.size = sf::Vector2f(Constants::CELL_SIZE - 2.f, Constants::LANE_HEIGHT - 2.f);
    }
}
#pragma endregion

#pragma region GET, SET
Constants::StaticObjectType Rock::getType() const { return Constants::StaticObjectType::Rock; }

void Rock::setPosition(const sf::Vector2f& pos)
{
    // 1. Khóa Origin về (0,0) và Scale số dương, triệt tiêu lật ảnh sai vị trí
    sf::Vector2f cur_scale = sprite.getScale();
    sprite.setScale({std::abs(cur_scale.x), std::abs(cur_scale.y)});
    sprite.setOrigin({0.f, 0.f});

    // 2. Cập nhật vị trí lớp cha
    StaticObject::setPosition(pos);

    // 3. Ép Hitbox nằm trọn vẹn 100% trong ranh giới ô Cell
    if (current_lane != nullptr)
    {
        float cell_x = position.x - (Constants::CELL_SIZE - getWidthSprite()) / 2.f;
        float lane_y = current_lane->getPosition().y;

        hitbox.position = sf::Vector2f(cell_x + 1.f, lane_y + 1.f);
        hitbox.size = sf::Vector2f(Constants::CELL_SIZE - 2.f, Constants::LANE_HEIGHT - 2.f);
    }
}

#pragma endregion

#pragma region OTHER
// keep the rock vertically locked to its lane as the map scrolls along Oy
void Rock::updateState(float delta_time) { setPosition(getPosition()); }
#pragma endregion
#pragma endregion
