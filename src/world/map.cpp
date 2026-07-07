#include "../include/world/map.h"

Map::Map()
    : lane_height(Constants::LANE_HEIGHT), highest_y(0), current_level(1)
{
    if (!road_texture.loadFromFile("assets/textures/cell.png"))
    {
        std::cerr << "Error loading road texture!\n";
    }
    if (!grass_texture.loadFromFile("assets/textures/grass.png"))
    {
        std::cerr << "Error loading grass texture!\n";
    }
    if (!safe_texture.loadFromFile("assets/textures/grass.png"))
    {
        std::cerr << "Error loading grass texture!\n";
    }
}

Map::~Map()
{
    lanes.clear();
}

void Map::initMap(float start_y)
{
    highest_y = start_y;
    for (int i = 0; i < 3; i++)
    {
        spawnLaneAsRequire(Constants::LaneType::Safe);
    }
    for (int i = 3; i < Constants::NUMBER_OF_LANES; i++)
    {
        spawnLane();
    }
}

void Map::spawnLane()
{
    static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<> Rand(0, 2);

    Constants::LaneType type;
    //      (Rand(rng) == 0) ? Constants::LaneType::Grass : Constants::LaneType::Road;

    int getRandom = Rand(rng);
    if (getRandom == 0)
    {
        type = Constants::LaneType::Grass;
    }
    else if (getRandom == 1)
    {
        type = Constants::LaneType::Road;
    }
    else
    {
        type = Constants::LaneType::Safe;
    }

    int direction = (Rand(rng) == 0) ? 1 : -1;

    sf::Vector2f spawn_position(0.f, highest_y);

    std::unique_ptr<Lane> new_lane =
        std::make_unique<Lane>(spawn_position, current_level, direction, type);

    lanes.push_front(std::move(new_lane));
    highest_y -= lane_height;
}

void Map::spawnLaneAsRequire(Constants::LaneType type)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> Rand(0, 1);
    int direction = (Rand(gen) == 0) ? 1 : -1;

    sf::Vector2f spawn_position(0.f, highest_y);
    auto new_lane =
        std::make_unique<Lane>(spawn_position, current_level, direction, type);

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

        if (lanes.size() % 10 == 0)
        {
            current_level++;
        }
    }

    // Cập nhật xe chạy (Đã vá chuẩn C++)
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

    current_level = 1;

    initMap(start_y);
}
