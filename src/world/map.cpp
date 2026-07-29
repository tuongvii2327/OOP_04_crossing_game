#include "world/map.h"
#include "entities/people.h"
#include "entities/wood.h"
#include "utils/resource_manager.h"
#include "utils/serialization.h"
#include <algorithm>
#include <cmath>

// Factory: create the correct Lane subclass for the given type
std::unique_ptr<Lane> Map::createLane(Constants::LaneType type, const sf::Vector2f& position,
                                      int level, int direction, bool spawn_obstacles)
{
    switch (type)
    {
    case Constants::LaneType::Grass:
        return std::make_unique<LaneGrass>(position, level, direction);
    case Constants::LaneType::Road:
        return std::make_unique<LaneRoad>(position, level, direction);
    case Constants::LaneType::River:
        return std::make_unique<LaneRiver>(position, level, direction);
    case Constants::LaneType::Safe:
    default:
        return std::make_unique<LaneSafe>(position, level, direction, spawn_obstacles);
    }
}

Map::Map(SharedContext* context)
    : lane_height(Constants::LANE_HEIGHT), highest_y(0.f), context_(context)
{
}

Map::~Map() { lanes.clear(); }

void Map::initMap(float start_y)
{
    highest_y = start_y;
    for (int i = 0; i < 3; i++)
    {
        spawnLaneAsRequire(Constants::LaneType::Safe,
                           false); // Initialize 3 safe lanes at first without obstacles (rocks)
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

    std::vector<double> weights; // Thứ tự trọng số tương ứng: [Grass, Road, Safe, River]

    int current_level = context_->data->current_level;
    if (current_level >= 10)
    {
        weights = {25.0, 50.0, 5.0, 20.0}; // 25% grass, 50% road, 5% safe, 20% river
    }
    else if (current_level >= 6)
    {
        weights = {25.0, 45.0, 10.0, 20.0}; // 25% grass, 45% road, 10% safe, 20% river
    }
    else if (current_level >= 3)
    {
        weights = {30.0, 35.0, 15.0, 20.0}; // 30% grass, 35% road, 15% safe, 20% river
    }
    else
    {
        weights = {35.0, 30.0, 20.0, 15.0}; // 35% grass, 30% road, 20% safe, 15% river
    }

    std::discrete_distribution<int> dist(weights.begin(), weights.end());

    const Constants::LaneType lane_mapping[] = {
        Constants::LaneType::Grass, // 0
        Constants::LaneType::Road,  // 1
        Constants::LaneType::Safe,  // 2
        Constants::LaneType::River  // 3
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

        /*if (duplicate_lane == 1 && type == Constants::LaneType::Safe)
        {
            break;
        }
        else if ()*/
        if ((type == Constants::LaneType::Safe && duplicate_lane == 1) ||
            (type != Constants::LaneType::Safe && duplicate_lane <= 3))
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

void Map::spawnLaneAsRequire(Constants::LaneType type, bool spawn_obstacles)
{

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> Rand(0, 1);
    int direction = (Rand(gen) == 0) ? 1 : -1;

    sf::Vector2f spawn_position(0.f, highest_y);
    auto new_lane =
        createLane(type, spawn_position, context_->data->current_level, direction, false);

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

    // Cập nhật xe chạy
    for (const auto& lane : lanes)
    {
        lane->updateState(delta_time);
    }
}

void Map::render(sf::RenderWindow& window) const
{
    for (const auto& lane : lanes)
    {
        lane->render(window);
    }
}

bool Map::checkLethalCollision(const sf::FloatRect& player_hitbox) const
{
    for (const auto& lane : lanes)
    {
        if (lane->checkLethalCollision(player_hitbox))
        {
            return true;
        }
    }
    return false;
}

sf::Vector2f Map::getWoodVelocityAt(const sf::FloatRect& player_hitbox) const
{
    for (const auto& lane : lanes)
    {
        if (lane->getType() != Constants::LaneType::River)
            continue;
        const LaneRiver* river = dynamic_cast<const LaneRiver*>(lane.get());
        if (!river)
            continue;
        sf::Vector2f v = river->getWoodVelocityAt(player_hitbox);
        if (v.x != 0.f || v.y != 0.f)
            return v;
    }
    return {0.f, 0.f};
}

void Map::snapPlayerToGrid(People* player) const
{
    if (!player || player->isDead())
        return;

    // Sử dụng target_position để kiểm tra làn đường đích đến, vì hitbox.position hiện tại vẫn ở
    // điểm bắt đầu nhảy
    sf::FloatRect hitbox = player->getHitBox();
    float target_y = player->getTargetPosition().y + player->getHitBoxOffset().y;

    const Lane* current_lane = nullptr;
    for (const auto& lane : lanes)
    {
        float lane_top = lane->getPosition().y;
        float lane_bottom = lane_top + Constants::LANE_HEIGHT;
        if (target_y >= lane_top && target_y <= lane_bottom)
        {
            current_lane = lane.get();
            break;
        }
    }

    if (!current_lane)
        return;

    float current_center_x = player->getTargetPosition().x + player->getWidthSprite() / 2.f;

    if (current_lane->getType() == Constants::LaneType::River)
    {
        const LaneRiver* river = dynamic_cast<const LaneRiver*>(current_lane);
        if (river)
        {
            // tạo hitbox giả lập tại vị trí đích để kiểm tra va chạm với Wood
            sf::FloatRect target_hitbox = hitbox;
            target_hitbox.position.x = player->getTargetPosition().x + player->getHitBoxOffset().x;
            target_hitbox.position.y = target_y;

            Wood* wood = river->getWoodAt(target_hitbox);
            if (wood)
            {
                float wood_x = wood->getPosition().x;
                float cell0_center = wood_x + Constants::CELL_SIZE / 2.f;
                float cell1_center = wood_x + Constants::CELL_SIZE + Constants::CELL_SIZE / 2.f;

                float dist0 = std::abs(current_center_x - cell0_center);
                float dist1 = std::abs(current_center_x - cell1_center);

                float target_center_x = (dist0 < dist1) ? cell0_center : cell1_center;
                float new_x = target_center_x - player->getWidthSprite() / 2.f;

                player->snapTargetX(new_x);
                return;
            }
        }
    }

    // Default static grid snap
    float relative_x = player->getTargetPosition().x - Constants::GAME_PLAY_BAR_WIDTH;
    int column = static_cast<int>(std::round(relative_x / Constants::CELL_SIZE));

    int total_cols = static_cast<int>(Constants::LANE_WIDTH / Constants::CELL_SIZE);
    column = std::max(0, std::min(column, total_cols - 1));

    float new_x = Constants::GAME_PLAY_BAR_WIDTH + column * Constants::CELL_SIZE +
                  (Constants::CELL_SIZE - player->getWidthSprite()) / 2.f;
    player->snapTargetX(new_x);
}

void Map::reset(float start_y)
{
    lanes.clear();

    total_lanes_spawned = 0;
    initMap(start_y);
}

// ----- SERIALIZE (SAVE MAP & LANES) ----
void Map::serialize(std::ostream& os) const
{
    // 1. Write map management properties
    Serialization::write(os, highest_y);
    Serialization::write(os, lane_height);
    Serialization::write(os, total_lanes_spawned);

    // 2. Write lane count
    size_t lanes_count = lanes.size();
    Serialization::write(os, lanes_count);

    // 3. Each lane serializes itself (type is written first by Lane::serialize)
    for (const auto& lane : lanes)
    {
        lane->serialize(os);
    }
}

// ---- DESERIALIZE (LOAD MAP & LANES) ----
void Map::deserialize(std::istream& is)
{
    // 1. Read map management properties
    Serialization::read(is, highest_y);
    Serialization::read(is, lane_height);
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
        case Constants::LaneType::River:
            lane = std::make_unique<LaneRiver>();
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

int Map::checkCoinCollision(const sf::FloatRect& player_hitbox)
{
    int coins_collected = 0;
    for (const auto& lane : lanes)
    {
        coins_collected += lane->checkCoinCollision(player_hitbox);
    }
    return coins_collected;
}

bool Map::checkBlockedCollision(const sf::FloatRect& target_hitbox) const
{
    for (const auto& lane : lanes)
    {
        if (lane->checkBlockedCollision(target_hitbox))
        {
            return true; // Có vật cản trên map
        }
    }
    return false;
}