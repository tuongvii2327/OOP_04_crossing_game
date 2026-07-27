#include "world/lane.h"

#include "entities/dynamic_object.h"
#include "utils/resource_manager.h"
#include "utils/serialization.h"

#include "entities/bird.h"
#include "entities/car.h"
#include "entities/rabbit.h"
#include "entities/truck.h"

#include <limits>
#include "../include/utils/serialization.h"

#pragma region PROTECTED
// move position by offset, update sprite position
void Lane::moveBy(sf::Vector2f offset)
{
    setPosition(position + offset);
}

// scale sprite up to fill LANE_WIDTH x LANE_HEIGHT if it's smaller
void Lane::adjustSprite()
{
    if (getWidthSprite() == 0.f || getHeightSprite() == 0.f)
        return;

    float scaleX = Constants::LANE_WIDTH / sprite.getLocalBounds().size.x;
    float scaleY = Constants::LANE_HEIGHT / sprite.getLocalBounds().size.y;
    sprite.setScale({scaleX, scaleY});
}

// random int in [start, end]
int Lane::randomRange(int start, int end)
{
    static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> dist(start, end);
    return static_cast<int>(dist(rng));
}

int Lane::pickTextureVariant(const Constants::LaneType& type)
{
    return randomRange(1, Constants::getNumberOfTextureVariant(type));
}

void Lane::seedCoins()
{
    // Tỷ lệ 50% mỗi làn đường xuất hiện 1 đồng xu
    if (randomRange(0, 9) >= 5)
        return;

        int total_cols = static_cast<int>(Constants::LANE_WIDTH / Constants::CELL_SIZE);
        int col = randomRange(1, std::max(1, total_cols - 2)); 
    
        // Truyền tọa độ góc trên-bên trái của ô Cell
        float cell_x = position.x + (col * Constants::CELL_SIZE);
        float cell_y = position.y;

    auto coin = std::make_unique<Coin>(sf::Vector2f{cell_x, cell_y}, this);
    list_static.push_back(std::move(coin));
}
#pragma endregion

#pragma region PUBLIC
// texture, direction, level, position, active state are set here;
// subclasses call seedObjects() and seedTraffic() in their own constructors
Lane::Lane(const sf::Vector2f& init_position, const int& init_level, const int& init_direction, const Constants::LaneType& init_type)
    : texture_variant(pickTextureVariant(init_type)), sprite(Resource().getLaneTexture(init_type, texture_variant))
{
    adjustSprite(); // <-- Kéo giãn sprite ngay khi làn đường vừa được tạo
    setDirection(init_direction);
    setLevel(init_level);
    setPosition(init_position);
    setActiveState(1);

    seedCoins();
}

Lane::~Lane() = default;

sf::Sprite Lane::getSprite() const
{
    return sprite;
}

// swap texture, then rescale sprite to fit the lane
void Lane::setSprite(const sf::Texture &texture)
{
    sprite.setTexture(texture, 1);
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

int Lane::getTextureVariant() const
{
    return texture_variant;
}

void Lane::setTextureVariant(int new_variant)
{
    int max_new_variant = Constants::getNumberOfTextureVariant(getType());
    if (new_variant < 1 || new_variant > max_new_variant)
        new_variant = pickTextureVariant(getType()); // fallback an toàn, không ném exception
    texture_variant = new_variant;
    setSprite(Resource().getLaneTexture(getType(), texture_variant));
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
    for (const auto &s : list_static) 
    {
        s->render(window);
    }
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

int Lane::checkCoinCollision(const sf::FloatRect &player_hitbox) 
{
    int collected_count = 0;
    for (auto &s : list_static)
    {
        if (auto coin = dynamic_cast<Coin *>(s.get()))
        {
            if (!coin->isCollected() && coin->isColliding(player_hitbox))
            {
                coin->collect(); // Đánh dấu đã nhặt
                collected_count++;
            }
        }
    }
    return collected_count;
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
    Serialization::write(os, texture_variant);
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

    // 4. Lưu danh sách STATIC OBJECTS
    size_t static_count = list_static.size();
    Serialization::write(os, static_count);
    for (const auto &s : list_static)
    {
        Serialization::write(os, s->getType());
        Serialization::write(os, s->getPosition().x);
        Serialization::write(os, s->getPosition().y);

        if (auto coin = dynamic_cast<Coin *>(s.get()))
        {
            Serialization::write(os, coin->isCollected()); // Lưu trạng thái đã bị ăn hay chưa
        }
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
    Serialization::read(is, texture_variant);
    Serialization::read(is, is_active);
    Serialization::read(is, num_active_object);

    setTextureVariant(texture_variant); //set variant and set sprite
    setPosition(position);

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

    // 4. Nạp lại danh sách static object
    list_static.clear();
    size_t static_count = 0;
    Serialization::read(is, static_count);
    for (size_t i = 0; i < static_count; ++i)
    {
        Constants::StaticObjectType s_type;
        float px = 0.f, py = 0.f;
        Serialization::read(is, s_type);
        Serialization::read(is, px);
        Serialization::read(is, py);

        if (s_type == Constants::StaticObjectType::Coin)
        {
            bool is_collected = false;
            Serialization::read(is, is_collected);
            auto coin = std::make_unique<Coin>(sf::Vector2f{px, py}, this);
            if (is_collected)
            {
                coin->collect();
            }
            list_static.push_back(std::move(coin));
        }
    }
}

#pragma endregion