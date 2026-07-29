#include "world/lane_grass.h"

#include "entities/bird.h"
#include "entities/rabbit.h"
#include <algorithm>
#include <limits>

LaneGrass::LaneGrass(const sf::Vector2f& init_position, const int& init_level,
                     const int& init_direction)
    : Lane(init_position, init_level, init_direction, Constants::LaneType::Grass)
{
    seedObjects();
}

LaneGrass::~LaneGrass() = default;

Constants::LaneType LaneGrass::getType() const { return Constants::LaneType::Grass; }

// populate list_dynamic with initial Bird/Rabbit objects, spread across the lane
void LaneGrass::seedObjects()
{
    int min_spawn = 1;
    int max_spawn = 2;

    if (level >= 10)
    {
        min_spawn = 3;
        max_spawn = Constants::MAX_OBJECT_PER_LANE;
    }
    else if (level >= 3)
    {
        min_spawn = 2;
        max_spawn = 3;
    }
    else
    {
        min_spawn = 1;
        max_spawn = 2;
    }
    int count = std::min(randomRange(min_spawn, max_spawn),
                         static_cast<int>(Constants::MAX_OBJECT_PER_LANE));

    const float min_gap = Constants::CELL_SIZE * 1.5f;

    for (int i = 0; i < count; ++i)
    {
        Constants::ObjectColor color = static_cast<Constants::ObjectColor>(
            randomRange(0, static_cast<int>(Constants::ObjectColor::None) - 1));

        std::unique_ptr<DynamicObject> obj;
        obj = (randomRange(0, 1) == 0)
                  ? std::unique_ptr<DynamicObject>(std::make_unique<Bird>(color, this))
                  : std::unique_ptr<DynamicObject>(std::make_unique<Rabbit>(color, this));

        float obj_width = obj->getWidthSprite();
        float lane_left = position.x, lane_right = position.x + Constants::LANE_WIDTH - obj_width;
        int range = lane_right - lane_left;
        if (range < 0)
        {
            range = 0;
        }
        float x = lane_left;
        bool placed = false;
        for (int tryy = 0; tryy < 10; tryy++)
        {
            x = lane_left + randomRange(0, range);
            bool overlap = false;
            for (auto& other : list_dynamic)
            {
                float other_x = other->getPosition().x;
                float other_width = other->getWidthSprite();
                if (x < other_x + other_width + min_gap && x + other_width + min_gap > other_x)
                {
                    overlap = true;
                    break;
                }
            }
            if (!overlap)
            {
                placed = true;
                break;
            }
        }

        obj->setPosition({x, 0});
        list_dynamic.push_back(std::move(obj));
    }

    std::sort(
        list_dynamic.begin(), list_dynamic.end(),
        [this](const std::unique_ptr<DynamicObject>& a, const std::unique_ptr<DynamicObject>& b)
        { return a->getPosition().x * direction > b->getPosition().x * direction; });
    num_active_object = static_cast<int>(list_dynamic.size());
}

// probabilistically spawn (or reuse a recycled inactive animal) once the lane
// has room and the most-recently-entered object has cleared the entry edge
void LaneGrass::spawnObject()
{
    int max_allowed = 2;
    if (level >= 10)
    {
        max_allowed = Constants::MAX_OBJECT_PER_LANE;
    }
    else if (level >= 3)
    {
        max_allowed = 3;
    }

    if (num_active_object >= max_allowed)
        return;
    if (list_dynamic.empty())
        return;

    int spawn_rate_chance = (level >= 6) ? 1 : 2;
    if (randomRange(0, spawn_rate_chance) != 0)
        return;

    DynamicObject* last_obj = list_dynamic.back().get();
    float lane_left = position.x;
    float lane_right = position.x + Constants::LANE_WIDTH;
    float entered_distance =
        (direction == 1) ? (last_obj->getPosition().x - lane_left)
                         : (lane_right - (last_obj->getPosition().x + last_obj->getWidthSprite()));
    if (entered_distance < static_cast<float>(Constants::CELL_SIZE))
        return;

    Constants::ObjectColor exp_color = static_cast<Constants::ObjectColor>(
        randomRange(0, static_cast<int>(Constants::ObjectColor::None) - 1));
    bool is_spawn = false;

    Constants::DynamicObjectType exp_type = static_cast<Constants::DynamicObjectType>(
        randomRange(static_cast<int>(Constants::DynamicObjectType::Bird),
                    static_cast<int>(Constants::DynamicObjectType::Rabbit)));

    // reuse an inactive object (same type+color) sitting in the exit prefix
    for (auto o = list_dynamic.begin(); o != list_dynamic.end() && !(*o)->isActive();)
    {
        if ((*o)->getType() == exp_type && (*o)->getColor() == exp_color)
        {
            Animal* target = dynamic_cast<Animal*>(o->get());
            if (target)
            {
                target->setActiveState(true);
                target->setPosition(target->calcSpawnPosition());
                target->setSpeed(target->randomSpeed(exp_type));
                target->setVelocity({target->getSpeed() * direction, 0.f});
                target->setMoveState(0);
                target->setMoveState(1);
                std::unique_ptr<DynamicObject> moved = std::move(*o);
                o = list_dynamic.erase(o);
                list_dynamic.push_back(std::move(moved));
                is_spawn = true;
            }
            break;
        }
        ++o;
    }
    if (!is_spawn)
    {
        if (exp_type == Constants::DynamicObjectType::Bird)
            list_dynamic.push_back(std::make_unique<Bird>(exp_color, this));
        else
            list_dynamic.push_back(std::make_unique<Rabbit>(exp_color, this));
    }
}

// closest active animal ahead of `animal` (per lane direction) that is either
// stopped or moving no faster than `animal`
Animal* LaneGrass::getNearestAnimalAhead(const Animal& animal)
{
    Animal* nearest = nullptr;
    float nearest_gap = std::numeric_limits<float>::max();
    for (auto& obj : list_dynamic)
    {
        if (!obj->isActive())
            continue;
        Animal* a = dynamic_cast<Animal*>(obj.get());
        if (!a)
            continue;
        float gap = (a->getPosition().x - animal.getPosition().x) * direction;
        if (gap <= 0.f)
            continue; // behind or same position
        bool is_obstacle =
            !a->isMove() || a->getVelocity().x * direction <= animal.getVelocity().x * direction;
        if (is_obstacle && gap < nearest_gap)
        {
            nearest_gap = gap;
            nearest = a;
        }
    }
    return nearest;
}

void LaneGrass::serialize(std::ostream& os) const
{
    // Grass lanes have no extra data beyond what the base class writes
    Lane::serialize(os);
}

void LaneGrass::deserialize(std::istream& is)
{
    // Grass lanes have no extra data beyond what the base class reads
    Lane::deserialize(is);
}
