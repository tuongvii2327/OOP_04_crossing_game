#pragma once

#include "lane.h"

class Animal;

class LaneGrass : public Lane
{
protected:
    void seedObjects() override;

public:
    LaneGrass(const sf::Vector2f& init_position = {0, 0}, const int& init_level = 1,
              const int& init_direction = 1);
    ~LaneGrass() override;

    Constants::LaneType getType() const override;
    void spawnObject() override;

    Animal* getNearestAnimalAhead(const Animal& animal);

    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
};
