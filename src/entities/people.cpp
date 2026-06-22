#include "../include/entities/people.h"
#include "../include/utils/constants.h"
#include <iostream>

People::People()
    : Object(Constants::ObjectType::People, Constants::ObjectColor::None, nullptr),
      is_dead(false),
      score(0),
      step_size(50.0f)
{
    resetPosition();
}

People::People(const sf::Texture &texture)
    : Object(Constants::ObjectType::People, Constants::ObjectColor::None, nullptr),
      is_dead(false),
      score(0),
      step_size(50.0f)
{
    setSprite(texture);
    resetPosition();
}

People::People(float x, float y)
    : Object(Constants::ObjectType::People, Constants::ObjectColor::None, nullptr),
      is_dead(false),
      score(0),
      step_size(50.0f)
{
    position = sf::Vector2f(x, y);
    sprite.setPosition(position);
    setHitbox();
}

Constants::ObjectType People::getType() const
{
    return Constants::ObjectType::People;
}

Constants::ObjectColor People::getColor() const
{
    return Constants::ObjectColor::None;
}

void People::updateState(float delta_time)
{
    if (is_dead || !is_active)
        return;

    // Nơi xử lý logic đếm lùi frame chuyển động (Animation) sau này
}

bool People::isColliding(const sf::FloatRect &other_hitbox) const
{
    if (is_dead || !is_active)
        return false;
    return hitbox.findIntersection(other_hitbox).has_value();
}

void People::move(float dx, float dy)
{
    if (is_dead)
        return;

    position.x += dx;
    position.y += dy;
    sprite.setPosition(position);
    setHitbox();
}

bool People::tryToMove(float dx, float dy, float camera_top_y)
{
    if (is_dead)
        return false;

    float target_x = position.x + dx;
    float target_y = position.y + dy;

    if (target_x < 0 || (target_x + getWidthSprite()) > Constants::WINDOW_WIDTH ||
        target_y < camera_top_y || target_y > (camera_top_y + Constants::WINDOW_HEIGHT))
    {
        return false;
    }

    move(dx, dy);
    return true;
}

void People::die()
{
    is_dead = true;
}

bool People::isDead() const
{
    return is_dead;
}

void People::resetPosition()
{
    float start_x = Constants::WINDOW_WIDTH / 2.f;

    float start_y = Constants::WINDOW_HEIGHT - (Constants::LANE_HEIGHT * 1.5f);

    position = sf::Vector2f(start_x, start_y);
    sprite.setPosition(position);
    setHitbox();

    is_dead = false;
}

void People::addScore()
{
    score++;
}

int People::getScore() const
{
    return score;
}

sf::FloatRect People::getBounds() const
{
    return sprite.getGlobalBounds();
}
