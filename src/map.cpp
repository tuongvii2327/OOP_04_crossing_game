#include "../include/world/map.h"


Map::Map() : lane_height(Constants::LANE_HEIGHT), highest_y(0), current_level(1){
    if (!road_texture.loadFromFile("assets/textures/cell.png")) {
        std::cerr << "Error loading road texture!\n";
    }
    if (!grass_texture.loadFromFile("assets/textures/grass.png")) {
        std::cerr << "Error loading grass texture!\n";
    }
}

Map::~Map() {
    for (Lane* x: lanes) {
        delete x;
    }
    lanes.clear();
}

void Map::initMap(float start_y) {
    highest_y = start_y;
    for (int i = 0; i < Constants::NUMBER_OF_LANES; i++) {
        spawnLane();
    }
}

void Map::spawnLane() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> Rand(0, 1);
    int get_random_number = Rand(gen);
    
    lane_type type;
    
    //Adjust the random spawn rate of different lane types.
    //Adjustments were made after consultation regarding the difficulty level.
    if (get_random_number == 0) {
        type = lane_type::GRASS;
    }
    else if (get_random_number == 1){
        type = lane_type::ROAD;
    }

    sf::Texture* selectedTexture;
    if (type == lane_type::GRASS) {
        selectedTexture = &grass_texture;
    }
    else if (type == lane_type::ROAD) {
        selectedTexture = &road_texture;
    }


    int direction = (Rand(gen) == 0);

    sf::Vector2f spawn_position(0, highest_y);
    Lane* new_lane = new Lane(type, *selectedTexture, spawn_position, current_level, direction);
    lanes.push_front(new_lane);
    highest_y -= lane_height;
}

void Map::popLane() {
    if (!lanes.empty()) {
        delete lanes.back();
        lanes.pop_back();
    }
}

void Map::update(float camera_top_y, float camera_bottom_y) {
    //Check delete if camera move up
    if (!lanes.empty()) {
        float bottom_lane_y = lanes.back()->getPosition().y;
        if (bottom_lane_y > camera_bottom_y) {
            popLane();
        }
    }
    //Check spawn 
    while (highest_y > camera_top_y - lane_height) {
        spawnLane();

        //maybe...
        if (lanes.size() % 10 == 0) {
            current_level++;
        }
    }
    //Vehicle runs
    //For (Lane* lane : lanes) {
    // lane->update(deltaTime)
    //}
}

void Map::render(sf::RenderWindow& window) const {
    for (auto it = lanes.begin(); it != lanes.end(); ++it) {
        (*it)->render(window);
    }
}
