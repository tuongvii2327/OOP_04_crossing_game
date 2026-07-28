#pragma once

#include "lane.h"
#include <deque>
#include <memory>

class StaticObject;

class LaneSafe : public Lane
{
private:
    std::deque<std::unique_ptr<StaticObject>> list_static;

protected:
    void seedObjects() override; // spawn rocks

public:
    LaneSafe(const sf::Vector2f &init_position = {0, 0},
             const int &init_level = 1,
             const int &init_direction = 1,
             bool spawn_obstacles = true);
    ~LaneSafe() override;

    Constants::LaneType getType() const override;
    void spawnObject() override; // no-op: safe lanes never spawn dynamic objects

    void updateState(float delta_time) override;
    void render(sf::RenderWindow &window) const override;
    void spawnLane(const sf::Vector2f &new_position,
                   const int &new_level,
                   const int &new_direction) override;

    void serialize(std::ostream &os) const override;
    void deserialize(std::istream &is) override;
};
