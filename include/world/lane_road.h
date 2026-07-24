#pragma once

#include "lane.h"
#include <memory>

class Traffic;
class Vehicle;

class LaneRoad : public Lane
{
private:
    std::unique_ptr<Traffic> traffic; // a Road lane has 0 or 1 traffic light

protected:
    void seedObjects() override;
    void seedTraffic();

public:
    LaneRoad(const sf::Vector2f &init_position = {0, 0},
             const int &init_level = 1,
             const int &init_direction = 1);
    ~LaneRoad() override;

    Constants::LaneType getType() const override;

    void updateState(float delta_time) override;
    void render(sf::RenderWindow &window) const override;
    void spawnLane(const sf::Vector2f &new_position,
                   const int &new_level,
                   const int &new_direction) override;
    void spawnObject() override;

    Traffic *getNearestTrafficAhead(const Vehicle &vehicle);
    Vehicle *getNearestVehicleAhead(const Vehicle &vehicle);

    void serialize(std::ostream &os) const override;
    void deserialize(std::istream &is) override;
};
