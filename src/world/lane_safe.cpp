#include "world/lane_safe.h"
#include "entities/rock.h"
#include "utils/serialization.h"

LaneSafe::LaneSafe(const sf::Vector2f &init_position, const int &init_level, const int &init_direction)
    : Lane(init_position, init_level, init_direction, Constants::LaneType::Safe)
{
    seedObjects();
}

LaneSafe::~LaneSafe() = default;

Constants::LaneType LaneSafe::getType() const
{
    return Constants::LaneType::Safe;
}

void LaneSafe::seedObjects()
{
    // Generate 0 to 2 rocks per safe lane
    int count = randomRange(0, 4);
    if (count == 0) return;

    std::vector<int> available_k;
    for (int k = -4; k <= 6; ++k)
    {
        available_k.push_back(k);
    }

    for (int i = 0; i < count; ++i)
    {
        if (available_k.empty()) break;
        
        // Pick a random distinct column
        int idx = randomRange(0, available_k.size() - 1);
        int k = available_k[idx];
        available_k.erase(available_k.begin() + idx);

        auto rock = std::make_unique<Rock>(this);
        
        // Player center starts at WINDOW_WIDTH / 2 (700) and moves by LANE_HEIGHT (105).
        // Align the rock center perfectly with the player's center.
        float player_center = (Constants::WINDOW_WIDTH / 2.f) + k * static_cast<float>(Constants::LANE_HEIGHT);
        float x = player_center - (rock->getWidthSprite() / 2.f);

        rock->setPosition({x, 0}); // position.y is handled by StaticObject
        list_static.push_back(std::move(rock));
    }
}

void LaneSafe::spawnObject()
{
    // no-op: safe lanes never spawn dynamic objects
}

void LaneSafe::updateState(float delta_time)
{
    Lane::updateState(delta_time);
    for (auto &o : list_static)
    {
        o->updateState(delta_time);
    }
}

void LaneSafe::render(sf::RenderWindow &window) const
{
    Lane::render(window);
    for (const auto &o : list_static)
    {
        o->render(window);
    }
}

void LaneSafe::spawnLane(const sf::Vector2f &new_position, const int &new_level, const int &new_direction)
{
    Lane::spawnLane(new_position, new_level, new_direction);
}

void LaneSafe::serialize(std::ostream &os) const
{
    Lane::serialize(os);
    
    size_t obj_count = list_static.size();
    Serialization::write(os, obj_count);
    
    for (const auto &obj : list_static)
    {
        Serialization::write(os, obj->getType());
        Serialization::write(os, obj->getPosition().x);
        Serialization::write(os, obj->getPosition().y);
    }
}

void LaneSafe::deserialize(std::istream &is)
{
    Lane::deserialize(is);
    
    list_static.clear();
    size_t obj_count = 0;
    Serialization::read(is, obj_count);
    
    for (size_t i = 0; i < obj_count; ++i)
    {
        Constants::StaticObjectType obj_type;
        float px = 0.f, py = 0.f;
        
        Serialization::read(is, obj_type);
        Serialization::read(is, px);
        Serialization::read(is, py);
        
        if (obj_type == Constants::StaticObjectType::Rock)
        {
            auto rock = std::make_unique<Rock>(this);
            rock->setPosition({px, py});
            list_static.push_back(std::move(rock));
        }
    }
}
