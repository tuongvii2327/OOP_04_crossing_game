#pragma once

#include <deque>
#include <memory>
#include <random>

#include <SFML/Graphics.hpp>

#include "../utils/constants.h"
#include "../utils/shared_context.h"

#include "lane.h"

class Map
{
private:
    // one lane is Lane, many Lanes is Map.=))
    std::deque<std::unique_ptr<Lane>> lanes;
    // Manage coordinates to know when to create/delete lanes.
    float highest_y;
    float lane_height;
    // current level
    int current_level;
    int duplicate_lane = 0;

    int total_lanes_spawned = 0;

    SharedContext *context_; // lưu trữ ngữ cảnh dùng chung

    std::mt19937 rng;

    void spawnLane();                                  // Generate random one lane in the top of map
    void spawnLaneAsRequire(Constants::LaneType type); // Generate type lane as required
    void popLane();                                    // Delete the oldest lane in the map

public:
    explicit Map(SharedContext *context = nullptr);
    ~Map();
    void initMap(float start_y);
    void update(float camera_top_y, float camera_bottom_y, float delta_time);
    void render(sf::RenderWindow &window) const;
    bool checkLethalCollision(const sf::FloatRect &player_hitbox) const;
    void reset(float start_y);

    // ---- Serialization & Deserialization functions ----
    void serialize(std::ostream &os) const;
    void deserialize(std::istream &is);
};
