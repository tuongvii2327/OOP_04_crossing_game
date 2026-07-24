#include "../include/entities/static_object.h"
#include "../include/utils/resource_manager.h"
#include "../include/world/lane.h"

#pragma region PRIVATE
// update hitbox position using position and hitbox_offset
void StaticObject::updateHitbox()
{
    hitbox.position = position + hitbox_offset;
}

// flip sprite depending on direction
void StaticObject::applyFacing()
{
    sf::Vector2f currentScale = sprite.getScale();
    float absX = std::abs(currentScale.x);

    float width = sprite.getLocalBounds().size.x;

    if (direction == -1)
    {
        sprite.setScale({-absX, currentScale.y});
        sprite.setOrigin({width, 0.f});
    }
    else
    {
        sprite.setScale({absX, currentScale.y});
        sprite.setOrigin({0.f, 0.f});
    }
}
#pragma endregion

#pragma region PROTECTED
// build hitbox: compute size, then offset, then position
void StaticObject::setHitbox()
{
    setHitboxSize();
    setHitboxOffset();
    updateHitbox();
}

// hitbox.size = sprite size * HITBOX_SIZE_RATIO
void StaticObject::setHitboxSize()
{
    hitbox.size = sprite.getGlobalBounds().size * Constants::HITBOX_SIZE_RATIO;
}

// hitbox_offset = (sprite size - hitbox.size) / 2
void StaticObject::setHitboxOffset()
{
    hitbox_offset = (sprite.getGlobalBounds().size - hitbox.size) / 2.f;
}

// direction taken from current_lane (1 if no lane attached)
int StaticObject::calcDirection() const
{
    return (current_lane != nullptr) ? current_lane->getDirection() : 1;
}

// reference into ResourceManager's cache (stays alive for the sprite's lifetime)
const sf::Texture &StaticObject::getTexture(const Constants::StaticObjectType &type, const Constants::ObjectColor &color) const
{
    return Resource().getStaticObjectTexture(type, color);
}

// scale size of sprite if its size is larger than CELL_SIZE
void StaticObject::adjustSprite()
{
    if (getWidthSprite() == 0.f || getHeightSprite() == 0.f)
        return;
    if (getWidthSprite() > Constants::LANE_WIDTH || getHeightSprite() > Constants::LANE_HEIGHT)
    {
        float scaleX = (Constants::LANE_WIDTH - Constants::VISUAL_DISTANCE) / getWidthSprite();
        float scaleY = (Constants::LANE_HEIGHT - Constants::VISUAL_DISTANCE) / getHeightSprite();
        float finalScale = std::min(scaleX, scaleY);
        sprite.setScale({finalScale, finalScale});
    }
}
#pragma endregion

#pragma region PUBLIC

#pragma region CONSTRUCTOR, DESTRUCTOR
// position, hitbox, direction are calculated & set automatically, sprite will be adjusted & apply facing
StaticObject::StaticObject(Constants::StaticObjectType init_type, const sf::Vector2f &init_position, Lane *init_lane, const Constants::ObjectColor &init_color)
    : sprite(Resource().getStaticObjectTexture(init_type, init_color)), object_color(init_color)
{
    adjustSprite();
    setLane(init_lane);
    setDirection(calcDirection());
    setPosition(init_position);
    setHitbox();
}
#pragma endregion

#pragma region GET, SET
sf::Vector2f StaticObject::getPosition() const
{
    return position;
}

void StaticObject::setPosition(const sf::Vector2f &pos)
{
    position = pos;
    if (current_lane != nullptr)
    {
        position.y = current_lane->getPosition().y + (Constants::LANE_HEIGHT - getHeightSprite()) / 2.f;
    }
    sprite.setPosition(position);
    updateHitbox();
}

sf::Sprite StaticObject::getSprite() const
{
    return sprite;
}

void StaticObject::setSprite(const sf::Texture &texture)
{
    sprite.setTexture(texture);
    adjustSprite();
    applyFacing();
    setHitbox(); // new texture may differ in size from the old one
}

sf::FloatRect StaticObject::getHitBox() const
{
    return hitbox;
}
// set hitbox in protected

sf::Vector2f StaticObject::getHitBoxOffset() const
{
    return hitbox_offset;
}
// set hitbox_offset in protected

Lane *StaticObject::getLane() const
{
    return current_lane;
}

void StaticObject::setLane(Lane *lane)
{
    current_lane = lane;
}

int StaticObject::getDirection() const
{
    return direction;
}

void StaticObject::setDirection(const int &direct)
{
    if (direct == 1 || direct == -1)
        direction = direct;
    applyFacing();
}

float StaticObject::getWidthSprite() const
{
    return sprite.getGlobalBounds().size.x;
}

float StaticObject::getHeightSprite() const
{
    return sprite.getGlobalBounds().size.y;
}

Constants::ObjectColor StaticObject::getColor() const
{
    return object_color;
}
#pragma endregion

#pragma region OTHER
// draw sprite to screen
void StaticObject::render(sf::RenderWindow &window) const
{
    window.draw(sprite);
}

// true if other hitbox intersects this object's hitbox
bool StaticObject::isColliding(const sf::FloatRect &other_hitbox) const
{
    return hitbox.findIntersection(other_hitbox).has_value();
}
#pragma endregion
#pragma endregion
