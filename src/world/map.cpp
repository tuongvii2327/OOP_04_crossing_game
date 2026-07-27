#include "world/map.h"
#include "utils/resource_manager.h"
#include "utils/serialization.h"

// Factory: create the correct Lane subclass for the given type
std::unique_ptr<Lane> Map::createLane(Constants::LaneType type, const sf::Vector2f &position, int level, int direction)
{
    switch (type)
    {
    case Constants::LaneType::Grass:
        return std::make_unique<LaneGrass>(position, level, direction);
    case Constants::LaneType::Road:
        return std::make_unique<LaneRoad>(position, level, direction);
    case Constants::LaneType::Safe:
    default:
        return std::make_unique<LaneSafe>(position, level, direction);
    }
}

Map::Map(SharedContext *context) : lane_height(Constants::LANE_HEIGHT), highest_y(0.f), current_level(1),context_(context){}

Map::~Map()
{
    lanes.clear();
}

void Map::initMap(float start_y)
{
    highest_y = start_y;
    for (int i = 0; i < 3; i++)
    {
        spawnLaneAsRequire(Constants::LaneType::Safe); // Initialize 3 safe lanes at first
    }
    for (int i = 3; i < Constants::NUMBER_OF_LANES; i++)
    {
        spawnLane();
    }
}

void Map::spawnLane() 
{
    static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<> DirRand(0, 1);

    std::vector<double> weights; // Thứ tự trọng số tương ứng: [Grass, Road, Safe]

    if (current_level >= 10)
    {
        weights = {30.0, 65.0, 5.0}; // 30% grass, 65% road, 5% safe lane
    }
    else if (current_level >= 6)
    {
        weights = {30.0, 55.0, 15.0}; // 30% grass, 55% road, 15% safe lane
    }
    else if (current_level >= 3)
    {
        weights = {35.0, 45.0, 20.0}; // 35% grass, 45% road, 20% safe lane
    }
    else
    {
        weights = {40.0, 40.0, 20.0}; // 40% grass, 40% road, 20% safe lane
    }

    std::discrete_distribution<int> dist(weights.begin(), weights.end());

    const Constants::LaneType lane_mapping[] = {
        Constants::LaneType::Grass, // 0
        Constants::LaneType::Road,  // 1
        Constants::LaneType::Safe   // 2
    };

    Constants::LaneType type;

    // Vòng lặp chống sinh quá 3 làn giống nhau liên tiếp
    do
    {
        int getRandom = dist(rng);
        type = lane_mapping[getRandom];

        if (!lanes.empty() && type == lanes.front()->getType())
        {
            duplicate_lane++;
        }
        else
        {
            duplicate_lane = 1;
        }

        if (duplicate_lane <= 3)
        {
            break;
        }

    } while (true);

    int direction = (DirRand(rng) == 0) ? 1 : -1;
    sf::Vector2f spawn_position(0.f, highest_y);

    std::unique_ptr<Lane> new_lane = createLane(type, spawn_position, current_level, direction);

    lanes.push_front(std::move(new_lane));
    highest_y -= lane_height;
    total_lanes_spawned++;
}

void Map::spawnLaneAsRequire(Constants::LaneType type)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> Rand(0, 1);
    int direction = (Rand(gen) == 0) ? 1 : -1;

    sf::Vector2f spawn_position(0.f, highest_y);
    auto new_lane = createLane(type, spawn_position, current_level, direction);

    if (!lanes.empty() && type == lanes.front()->getType())
    {
        duplicate_lane++;
    }
    else
    {
        duplicate_lane = 1;
    }

    lanes.push_front(std::move(new_lane));
    highest_y -= lane_height;
}

void Map::popLane()
{
    if (!lanes.empty())
    {
        lanes.pop_back();
    }
}

void Map::update(float camera_top_y, float camera_bottom_y, float delta_time)
{
    // Check delete if camera move up
    while (!lanes.empty())
    {
        float bottom_lane_y = lanes.back()->getPosition().y;
        if (bottom_lane_y >= camera_bottom_y)
        {
            popLane();
        }
        else
        {
            break;
        }
    }
    // Check spawn
    while (highest_y > camera_top_y - lane_height)
    {
        spawnLane();
    }

    if (context_ && context_->data)
    {
        int player_score = context_->data->current_score;
        current_level = player_score / 10;
    }

    // Cập nhật xe chạy
    for (const auto &lane : lanes)
    {
        lane->updateState(delta_time);
    }
}

void Map::render(sf::RenderWindow &window) const
{
    for (const auto &lane : lanes)
    {
        lane->render(window);
    }
}

bool Map::checkLethalCollision(const sf::FloatRect &player_hitbox) const
{
    for (const auto &lane : lanes)
    {
        if (lane->checkLethalCollision(player_hitbox))
        {
            return true;
        }
    }
    return false;
}

void Map::reset(float start_y)
{
    lanes.clear();

    current_level = 0;
    total_lanes_spawned = 0;
    initMap(start_y);
}

// ----- SERIALIZE (SAVE MAP & LANES) ----
void Map::serialize(std::ostream &os) const
{
    // 1. Write map management properties
    Serialization::write(os, highest_y);
    Serialization::write(os, lane_height);
    Serialization::write(os, current_level);
    Serialization::write(os, total_lanes_spawned);

    // 2. Write lane count
    size_t lanes_count = lanes.size();
    Serialization::write(os, lanes_count);

    // 3. Each lane serializes itself (type is written first by Lane::serialize)
    for (const auto &lane : lanes)
    {
        lane->serialize(os);
    }
}

// ---- DESERIALIZE (LOAD MAP & LANES) ----
void Map::deserialize(std::istream &is)
{
    // 1. Read map management properties
    Serialization::read(is, highest_y);
    Serialization::read(is, lane_height);
    Serialization::read(is, current_level);
    Serialization::read(is, total_lanes_spawned);

    // 2. Read lane count
    size_t lanes_count = 0;
    Serialization::read(is, lanes_count);

    // Clear old map data
    lanes.clear();

    // 3. For each lane: read its type first, create the correct subclass, then deserialize
    for (size_t i = 0; i < lanes_count; ++i)
    {
        // Read lane type to determine which subclass to create
        Constants::LaneType lane_type;
        Serialization::read(is, lane_type);

        // Create the correct subclass with default params (will be overwritten by deserialize)
        std::unique_ptr<Lane> lane;
        switch (lane_type)
        {
        case Constants::LaneType::Grass:
            lane = std::make_unique<LaneGrass>();
            break;
        case Constants::LaneType::Road:
            lane = std::make_unique<LaneRoad>();
            break;
        case Constants::LaneType::Safe:
        default:
            lane = std::make_unique<LaneSafe>();
            break;
        }

        // Deserialize reads remaining data (type already consumed above)
        lane->deserialize(is);

        lanes.push_back(std::move(lane));
    }
}

int Map::checkCoinCollision(const sf::FloatRect &player_hitbox)
{
    int coins_collected = 0;
    for (const auto &lane : lanes)
    {
        coins_collected += lane->checkCoinCollision(player_hitbox);
    }
    return coins_collected;
}
