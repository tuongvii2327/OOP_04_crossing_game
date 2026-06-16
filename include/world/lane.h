#pragma once

#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>

#include <SFML/Graphics.hpp>

#include "../utils/constants.h"

class DynamicObject;
class Vehicle;
class Traffic;

class Lane {
  sf::Sprite sprite;
  sf::Vector2f position;
  int level;
  int direction;
  std::vector<DynamicObject *> list_obj;
  std::vector<Traffic *> list_traffic;

protected:
  void adjustSprite();

public:
  Lane(const sf::Texture &texture, 
      const sf::Vector2f &init_position = {0, 0},
      const int &init_level = 1, 
      const int &init_direction = 1);
  ~Lane();

  sf::Sprite getSprite() const;
  void setSprite(const sf::Sprite &new_sprite);

  sf::Vector2f getPosition() const;
  void setPosition(const sf::Vector2f &pos);

  int getLevel() const;
  void setLevel(const int &new_level);

  int getDirection() const;
  void setDirection(const int &direct);

  void addObject(DynamicObject *obj);
  void addTraffic(Traffic *t);

  Traffic *getNearestTrafficAhead(const Vehicle &vehicle);
  Vehicle *getNearestVehicleAhead(const Vehicle &vehicle);

  void render(sf::RenderWindow &window) const;
};