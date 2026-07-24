#pragma once

#include <deque>
#include <memory>

#include "../utils/constants.h"
#include <SFML/Graphics.hpp>

// Foward declarations
class DynamicObject;
class StaticObject;
class Vehicle;
class Animal;
class Traffic;

class Lane
{
private:
    sf::Sprite sprite;
    sf::Vector2f position; // top left point of sprite
    int level = 1;         // level of game in current frame
    int direction;         // 1 = move right, -1 = move left
    bool is_active = 0;
    int num_active_object = 0;
    std::deque<std::unique_ptr<DynamicObject>> list_dynamic;
    std::deque<std::unique_ptr<StaticObject>> list_static;
    std::unique_ptr<Traffic> traffic; // a Road lane has 0 or 1 traffic light, never more
    Constants::LaneType type;

protected:
    void moveBy(sf::Vector2f offset);                                                   // move position by offset, update sprite position
    const sf::Texture &getTexture(const Constants::LaneType type, int direction) const; // cached lane texture
    void adjustSprite();                                                                // scale sprite to fit LANE_WIDTH x LANE_HEIGHT
    int randomRange(int start, int end);                                                // random int in [start, end]
    void seedObjects();                                                                 // populate list_obj with 2-4 initial objects matching the lane type
    void seedTraffic();                                                                 // populate traffic with 0 or 1 light (Road lanes only)

public:
    // contructor, destructor
    Lane(const sf::Vector2f &init_position = {0, 0}, const int &init_level = 1, const int &init_direction = 1, const Constants::LaneType &init_type = Constants::LaneType::Grass);
    ~Lane();

    // get, set
    sf::Sprite getSprite() const;
    void setSprite(const sf::Texture &texture);

    sf::Vector2f getPosition() const;
    void setPosition(const sf::Vector2f &pos);

    int getLevel() const;
    void setLevel(int new_level);

    int getDirection() const;
    void setDirection(int direct);

    bool isActive() const;
    void setActiveState(bool state);

    int getNumberOfActive() const;
    void setNumberOfActive(int new_num);

    float getWidthSprite() const;
    float getHeightSprite() const;

    Constants::LaneType getType() const;
    void setType(const Constants::LaneType &new_type);

    // Other
    void checkActive();                          // re-marks the lane active every frame (lanes behind the camera are removed by Map, not deactivated here)
    void updateState(float delta_time);          // advance traffic light + all objects, recount actives, then spawn
    void render(sf::RenderWindow &window) const; // draw lane sprite, objects, and traffic light

    void spawnLane(const sf::Vector2f &new_position, const int &new_level, const int &new_direction); // reuse this Lane instance for a newly scrolled-in row: reposition, flip direction/objects if needed, reshuffle the traffic light
    void spawnObject();                                                                               // probabilistically spawn (or reuse a recycled) object once the lane has room and the last object cleared the entry edge

    Traffic *getNearestTrafficAhead(const Vehicle &vehicle);             // the lane's single traffic light if it's red/yellow, ahead of vehicle, and within its safe distance
    Vehicle *getNearestVehicleAhead(const Vehicle &vehicle);             // the vehicle immediately ahead of `vehicle` in list_obj, if it is slower
    Animal *getNearestAnimalAhead(const Animal &animal);                 // the closest stopped/slower animal ahead of `animal`
    bool checkLethalCollision(const sf::FloatRect &player_hitbox) const; // true if any active lethal object's hitbox intersects player_hitbox
};
