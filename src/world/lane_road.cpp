#include "world/lane_road.h"

#include "entities/car.h"
#include "entities/traffic_light.h"
#include "entities/truck.h"
#include "utils/resource_manager.h"
#include "utils/serialization.h"

#include <limits>
#include <algorithm>

LaneRoad::LaneRoad(const sf::Vector2f &init_position, const int &init_level, const int &init_direction)
    : Lane(init_position, init_level, init_direction, Constants::LaneType::Road)
{
    seedObjects();
    seedTraffic();
}

LaneRoad::~LaneRoad() = default;

Constants::LaneType LaneRoad::getType() const
{
    return Constants::LaneType::Road;
}

// populate list_dynamic with initial Car/Truck objects, spread across the lane
void LaneRoad::seedObjects()
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
                  ? std::unique_ptr<DynamicObject>(std::make_unique<Car>(color, this))
                  : std::unique_ptr<DynamicObject>(std::make_unique<Truck>(color, this));

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
            for (auto &other : list_dynamic)
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

    std::sort(list_dynamic.begin(), list_dynamic.end(),
              [this](const std::unique_ptr<DynamicObject> &a, const std::unique_ptr<DynamicObject> &b)
              {
                  return a->getPosition().x * direction > b->getPosition().x * direction;
              });
    num_active_object = static_cast<int>(list_dynamic.size());
}

// populate traffic with 0 or 1 traffic light
void LaneRoad::seedTraffic()
{
    const float margin = 2.f * Constants::CELL_SIZE;
    const float gap_min = 3.f * Constants::CELL_SIZE;
    const float usable = Constants::LANE_WIDTH - 2.f * margin;

    int count = randomRange(0, 1);
    if (count == 0)
        return;

    int slot = usable / (count + 1);
    float x = position.x + margin + slot;
    Constants::TrafficLightColor color =
        (randomRange(0, 1) == 0) ? Constants::TrafficLightColor::Green
                                 : Constants::TrafficLightColor::Red;
    traffic = std::make_unique<Traffic>(this, color);
    traffic->setPosition({x, 0});
}

// advance traffic light + base class logic (objects, recount, spawn)
void LaneRoad::updateState(float delta_time)
{
    if (traffic)
        traffic->updateState(delta_time);
    Lane::updateState(delta_time);
}

// draw lane sprite + objects (base), then traffic light
void LaneRoad::render(sf::RenderWindow &window) const
{
    window.draw(sprite);
    if (traffic) // must draw traffic before vehicle
        traffic->render(window);
    for (const auto& o : list_dynamic)
    {
        o->render(window);
    } 
}

// reuse lane: base reposition + shuffle traffic light
void LaneRoad::spawnLane(const sf::Vector2f &new_position, const int &new_level, const int &new_direction)
{
    Lane::spawnLane(new_position, new_level, new_direction);

    // flip traffic direction if needed
    if (traffic && new_direction != direction)
    {
        traffic->setDirection(new_direction);
    }

    // shuffle the lane's existing traffic light to a new random spot
    if (traffic)
    {
        float lane_left = position.x;
        int lo = static_cast<int>(0.1f * Constants::LANE_WIDTH);
        int hi = static_cast<int>(0.9f * Constants::LANE_WIDTH);
        traffic->setPosition(
            {lane_left + static_cast<float>(randomRange(lo, hi)), position.y});
    }
}

// probabilistically spawn (or reuse a recycled inactive vehicle) once the lane
// has room and the most-recently-entered object has cleared the entry edge
void LaneRoad::spawnObject()
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

    DynamicObject *last_obj = list_dynamic.back().get();
    float lane_left = position.x;
    float lane_right = position.x + Constants::LANE_WIDTH;
    float entered_distance = (direction == 1)
                                 ? (last_obj->getPosition().x - lane_left)
                                 : (lane_right - (last_obj->getPosition().x +
                                                  last_obj->getWidthSprite()));
    if (entered_distance < static_cast<float>(Constants::CELL_SIZE))
        return;

    Constants::ObjectColor exp_color = static_cast<Constants::ObjectColor>(
        randomRange(0, static_cast<int>(Constants::ObjectColor::None) - 1));
    bool is_spawn = false;

    Constants::DynamicObjectType exp_type = static_cast<Constants::DynamicObjectType>(
        randomRange(static_cast<int>(Constants::DynamicObjectType::Car),
                    static_cast<int>(Constants::DynamicObjectType::Bird) - 1));

    for (auto o = list_dynamic.begin(); o != list_dynamic.end() && !(*o)->isActive();)
    {
        if ((*o)->getType() == exp_type && (*o)->getColor() == exp_color)
        {
            Vehicle *target = dynamic_cast<Vehicle *>(o->get());
            if (target)
            {
                target->setActiveState(true);
                target->setPosition(target->calcSpawnPosition());
                target->setBaseSpeed(target->randomSpeed(exp_type));
                target->setVelocity({target->getBaseSpeed() * direction, 0.f});
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
        if (exp_type == Constants::DynamicObjectType::Car)
            list_dynamic.push_back(std::make_unique<Car>(exp_color, this));
        else
            list_dynamic.push_back(std::make_unique<Truck>(exp_color, this));
    }
}

// the lane's single traffic light if it's red/yellow, ahead of vehicle, and
// within its safe distance
Traffic *LaneRoad::getNearestTrafficAhead(const Vehicle &vehicle)
{
    if (!traffic)
        return nullptr;

    Constants::TrafficLightColor color = traffic->getTrafficColor();
    if (color == Constants::TrafficLightColor::Green)
        return nullptr;

    float gap = vehicle.gapToObstacle(traffic->getPosition().x, Constants::getSizeInGame(Constants::StaticObjectType::Traffic).width);
    if (gap < 0 || gap > vehicle.getSafeDistance())
        return nullptr;

    return traffic.get();
}

// the vehicle immediately ahead of `vehicle` in list_dynamic, if it is slower
Vehicle *LaneRoad::getNearestVehicleAhead(const Vehicle &vehicle)
{
    for (auto it = list_dynamic.begin(); it != list_dynamic.end(); ++it)
    {
        if (it->get() != &vehicle)
            continue;
        if (it == list_dynamic.begin())
            return nullptr;
        auto ahead_it = std::prev(it);
        if (!(*ahead_it)->isActive())
            return nullptr;
        Vehicle *ahead = dynamic_cast<Vehicle *>(ahead_it->get());
        if (ahead && ahead->getVelocity().x * direction < vehicle.getVelocity().x * direction)
            return ahead;
        return nullptr;
    }
    return nullptr;
}

void LaneRoad::serialize(std::ostream &os) const
{
    // 1. Write base lane data
    Lane::serialize(os);

    // 2. Write traffic light data
    bool has_traffic = (traffic != nullptr);
    Serialization::write(os, has_traffic);
    if (has_traffic)
    {
        Serialization::write(os, traffic->getPosition().x);
        Serialization::write(os, traffic->getPosition().y);
        Serialization::write(os, traffic->getTrafficColor());
    }
}

void LaneRoad::deserialize(std::istream &is)
{
    // 1. Read base lane data
    Lane::deserialize(is);

    // 2. Read traffic light data
    bool has_traffic = false;
    Serialization::read(is, has_traffic);
    if (has_traffic)
    {
        float tx = 0.f, ty = 0.f;
        Constants::TrafficLightColor t_color;
        Serialization::read(is, tx);
        Serialization::read(is, ty);
        Serialization::read(is, t_color);

        traffic = std::make_unique<Traffic>(this, t_color);
        traffic->setPosition({tx, ty});
    }
    else
    {
        traffic.reset();
    }
}
