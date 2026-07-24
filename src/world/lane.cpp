#include "world/lane.h"

#include "entities/dynamic_object.h"
#include "utils/resource_manager.h"
#include "utils/serialization.h"

#include "entities/bird.h"
#include "entities/car.h"
#include "entities/rabbit.h"
#include "entities/truck.h"

#include <limits>

#pragma region PROTECTED
// move position by offset, update sprite position
void Lane::moveBy(sf::Vector2f offset)
{
    setPosition(position + offset);
}

// reference into ResourceManager's cache (stays alive for the sprite's lifetime)
const sf::Texture &Lane::getTexture(const Constants::LaneType type, int direction) const
{
    return Resource().getLaneTexture(type, direction);
}

// scale sprite up to fill LANE_WIDTH x LANE_HEIGHT if it's smaller
void Lane::adjustSprite()
{
    if (getWidthSprite() == 0.f || getHeightSprite() == 0.f)
        return;

    // Luôn luôn ép co giãn ảnh cho vừa bằng kích thước của làn đường (1260x105)
    float scaleX = Constants::LANE_WIDTH / getWidthSprite();
    float scaleY = Constants::LANE_HEIGHT / getHeightSprite();
    sprite.setScale({scaleX, scaleY});
}

// random int in [start, end]
int Lane::randomRange(int start, int end)
{
    static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> dist(start, end);
    return static_cast<int>(dist(rng));
}
#pragma endregion

#pragma region PUBLIC
// texture, direction, level, position, active state are set here;
// subclasses call seedObjects() and seedTraffic() in their own constructors
Lane::Lane(const sf::Vector2f &init_position, const int &init_level, const int &init_direction, const Constants::LaneType &init_type)
    : sprite(getTexture(init_type, init_direction))
{
    adjustSprite(); // <-- Kéo giãn sprite ngay khi làn đường vừa được tạo
    setDirection(init_direction);
    setLevel(init_level);
    setPosition(init_position);
    setActiveState(1);
}

Lane::~Lane() = default;

sf::Sprite Lane::getSprite() const
{
    return sprite;
}

// swap texture, then rescale sprite to fit the lane
void Lane::setSprite(const sf::Texture &texture)
{
    sprite.setTexture(texture);
    adjustSprite();
}

sf::Vector2f Lane::getPosition() const
{
    return position;
}

void Lane::setPosition(const sf::Vector2f &pos)
{
    position = pos;
    position.x = Constants::WINDOW_WIDTH - Constants::LANE_WIDTH; // lane is always right of the sidebar
    sprite.setPosition(position);
}

int Lane::getLevel() const
{
    return level;
}

void Lane::setLevel(int new_level)
{
    level = new_level;
}

int Lane::getDirection() const
{
    return direction;
}

void Lane::setDirection(int direct)
{
    if (direct == 1 || direct == -1)
        direction = direct;
}

bool Lane::isActive() const
{
    return is_active;
}

void Lane::setActiveState(bool state)
{
    is_active = state;
}

int Lane::getNumberOfActive() const
{
    return num_active_object;
}

void Lane::setNumberOfActive(int new_num)
{
    // Cố tình dùng min/max thay thế cho std::clamp để chống lỗi phiên bản C++
    num_active_object = std::min(std::max(new_num, 0), static_cast<int>(Constants::MAX_OBJECT_PER_LANE));
}

float Lane::getWidthSprite() const
{
    return sprite.getGlobalBounds().size.x;
}

float Lane::getHeightSprite() const
{
    return sprite.getGlobalBounds().size.y;
}
#pragma endregion

#pragma region OTHER
// re-marks the lane active every frame (lanes behind the camera are removed by Map, not deactivated here)
void Lane::checkActive()
{
    setActiveState(true);
}

// advance all objects, recount actives, then spawn
void Lane::updateState(float delta_time)
{
    for (auto &o : list_dynamic)
        o->updateState(delta_time);

    // recount active objects
    num_active_object = 0;
    for (auto &o : list_dynamic)
        if (o->isActive())
            ++num_active_object;

    checkActive();
    spawnObject();
}

// draw lane sprite, then every object
void Lane::render(sf::RenderWindow &window) const
{
    window.draw(sprite);
    for (const auto &o : list_dynamic)
    {
        o->render(window);
    }
}

// reuse this Lane instance for a newly scrolled-in row: reposition, flip
// direction/objects if the direction changed
void Lane::spawnLane(const sf::Vector2f &new_position, const int &new_level, const int &new_direction)
{
    setPosition(new_position);
    setLevel(new_level);
    if (new_direction != direction)
    {
        setDirection(new_direction);
        for (auto &o : list_dynamic)
        {
            o->setDirection(new_direction);
            o->setVelocity({-o->getVelocity().x, o->getVelocity().y});
        }
    }
}

// true if any active object with a Lethal effect is colliding with player_hitbox
bool Lane::checkLethalCollision(const sf::FloatRect &player_hitbox) const
{
    for (const auto &obj : list_dynamic)
    {
        if (!obj->isActive())
            continue;

        if (obj->getEffect() == Constants::ObjectEffect::Lethal && obj->isColliding(player_hitbox))
        {
            return true;
        }
    }
    return false;
}
#pragma endregion

#pragma region SAVE-LOAD

// ---- SERIALIZE (base class: lane props + dynamic objects) ----
void Lane::serialize(std::ostream &os) const
{
    // 1. Write lane type so deserializer knows which subclass to create
    Constants::LaneType t = getType();
    Serialization::write(os, t);

    // 2. Write basic lane properties
    Serialization::write(os, position.x);
    Serialization::write(os, position.y);
    Serialization::write(os, level);
    Serialization::write(os, direction);
    Serialization::write(os, is_active);
    Serialization::write(os, num_active_object);

    // 3. Write dynamic objects
    size_t obj_count = list_dynamic.size();
    Serialization::write(os, obj_count);
    for (const auto &obj : list_dynamic)
    {
        Serialization::write(os, obj->getType());
        Serialization::write(os, obj->getColor());
        Serialization::write(os, obj->getPosition().x);
        Serialization::write(os, obj->getPosition().y);
        Serialization::write(os, obj->getVelocity().x);
        Serialization::write(os, obj->getVelocity().y);
        Serialization::write(os, obj->isActive());
    }
}

// ---- DESERIALIZE (base class: lane props + dynamic objects) ----
// NOTE: The lane type has already been consumed by Map::deserialize
// to create the correct subclass. Do NOT read type here.
void Lane::deserialize(std::istream &is)
{
    // 1. Read basic lane properties (type already consumed by Map)
    Serialization::read(is, position.x);
    Serialization::read(is, position.y);
    Serialization::read(is, level);
    Serialization::read(is, direction);
    Serialization::read(is, is_active);
    Serialization::read(is, num_active_object);

    sprite.setTexture(Resource().getLaneTexture(getType(), direction));
    sprite.setPosition(position);

    // 2. Read dynamic objects
    list_dynamic.clear();
    size_t obj_count = 0;
    Serialization::read(is, obj_count);

    for (size_t i = 0; i < obj_count; ++i)
    {
        Constants::DynamicObjectType obj_type;
        Constants::ObjectColor obj_color;
        float px = 0.f, py = 0.f;
        float vx = 0.f, vy = 0.f;
        bool obj_active = false;

        Serialization::read(is, obj_type);
        Serialization::read(is, obj_color);
        Serialization::read(is, px);
        Serialization::read(is, py);
        Serialization::read(is, vx);
        Serialization::read(is, vy);
        Serialization::read(is, obj_active);

        // FACTORY PATTERN IMPLEMENT
        std::unique_ptr<DynamicObject> obj;
        switch (obj_type)
        {
        case Constants::DynamicObjectType::Car:
            obj = std::make_unique<Car>(obj_color, this);
            break;
        case Constants::DynamicObjectType::Truck:
            obj = std::make_unique<Truck>(obj_color, this);
            break;
        case Constants::DynamicObjectType::Bird:
            obj = std::make_unique<Bird>(obj_color, this);
            break;
        case Constants::DynamicObjectType::Rabbit:
            obj = std::make_unique<Rabbit>(obj_color, this);
            break;
        default:
            continue;
        }

        if (obj)
        {
            obj->setPosition({px, py});
            obj->setVelocity({vx, vy});
            obj->setActiveState(obj_active);
            list_dynamic.push_back(std::move(obj));
        }
    }
}

#pragma endregion