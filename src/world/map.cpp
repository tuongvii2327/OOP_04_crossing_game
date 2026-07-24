#include "world/map.h"
#include "utils/resource_manager.h"
#include "utils/serialization.h"

Map::Map(SharedContext *context)
    : lane_height(Constants::LANE_HEIGHT),
      highest_y(0.f),
      current_level(1),
      context_(context)
{
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

    std::unique_ptr<Lane> new_lane =
        std::make_unique<Lane>(spawn_position, current_level, direction, type);

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
    auto new_lane =
        std::make_unique<Lane>(spawn_position, current_level, direction, type);

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

// ----- ĐỊNH NGHĨA HÀM SERIALIZE (LƯU MAP & LANES) ----
void Map::serialize(std::ostream &os) const
{
    // 1. Lưu các thuộc tính quản lý tọa độ và cấp độ của Map
    Serialization::write(os, highest_y);
    Serialization::write(os, lane_height);
    Serialization::write(os, current_level);
    Serialization::write(os, total_lanes_spawned);

    // 2. Lưu số lượng Lane hiện tại
    size_t lanes_count = lanes.size();
    Serialization::write(os, lanes_count);

    // 3. Yêu cầu từng Lane tự Serialize dữ liệu của nó
    for (const auto &lane : lanes)
    {
        lane->serialize(os);
    }
}

// ---- ĐỊNH NGHĨA HÀM DESERIALIZE (NẠP LẠI MAP & LANES) ----
void Map::deserialize(std::istream &is)
{
    // 1. Đọc lại các thuộc tính Map
    Serialization::read(is, highest_y);
    Serialization::read(is, lane_height);
    Serialization::read(is, current_level);
    Serialization::read(is, total_lanes_spawned);

    // 2. Đọc số lượng Lane cần nạp
    size_t lanes_count = 0;
    Serialization::read(is, lanes_count);

    // Xóa dữ liệu bản đồ cũ
    lanes.clear();

    // 3. Khởi tạo lại và phục hồi dữ liệu cho từng Lane theo đúng thứ tự
    for (size_t i = 0; i < lanes_count; ++i)
    {
        // Tạo một Lane rỗng với các tham số mặc định trước
        auto lane = std::make_unique<Lane>();

        // Gọi hàm Deserialize của chính Lane đó để nạp đè dữ liệu từ file lên
        lane->deserialize(is);

        // Đẩy trở lại vào Deque map
        lanes.push_back(std::move(lane));
    }
}