#pragma once

#include "lane.h"

class Wood;

class LaneRiver : public Lane {
protected:
  void seedObjects() override;

public:
  LaneRiver(const sf::Vector2f &init_position = {0, 0},
            const int &init_level = 1, const int &init_direction = 1);
  ~LaneRiver() override;

  Constants::LaneType getType() const override;
  void spawnObject() override;

  // River is lethal: player dies if standing on water without being on a Wood
  bool checkLethalCollision(const sf::FloatRect &player_hitbox) const override;

  // Helper: returns the velocity of the Wood the player is standing on, or
  // {0,0} if none
  sf::Vector2f getWoodVelocityAt(const sf::FloatRect &player_hitbox) const;

  // the wood immediately ahead of `wood` in list_dynamic, if it is slower
  Wood *getNearestWoodAhead(const Wood &wood);

  void updateState(float delta_time) override; //call Lane::updateState and update frame, set new sprite
};
