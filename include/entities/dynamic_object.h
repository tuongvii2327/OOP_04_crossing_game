#pragma once

#pragma region INCLUDE
#include <iostream>
#include <random>

// include other file
#include "../utils/constants.h"

// Lib SFML, using file from vc17 x64 version
#include <SFML/Graphics.hpp>
// #include <SFML/Audio.hpp>
#pragma endregion

class Lane;

class DynamicObject {
private:
  void updateHitbox(); // reposition hitbox from position and offset
  void applyFacing(); // flip the sprite horizontally when direction == -1, keep it as-is when direction == 1

protected:
#pragma region VARIABLES
  sf::Vector2f position; // top left point of sprite
  sf::Vector2f velocity; // movement speed (px/s)
  sf::Sprite sprite;
  sf::FloatRect hitbox; // collision rectangle (top left point, width x height)
  sf::Vector2f hitbox_offset; // offset of hitbox top-left from sprite top-left
  bool is_active;             // inside window or not
  Lane *current_lane;         // lane this object currently belongs to
  int direction = 1;          // 1 = move right, -1 = move left
#pragma endregion

  void setHitbox(); // set offset, size and recalculate hitbox
  void setHitboxSize(); // HITBOX_SIZE_RATIO * sprite size
  void setHitboxOffset();

  void moveBy(sf::Vector2f offset);  // move by a relative offset, then sync sprite and hitbox
  float randomSpeed(Constants::DynamicObjectType type) const; // pick a random float from the speed range for 'type', scaled by level

  // calculate for variants
  int calcDirection() const;           // direction taken from current_lane
  sf::Vector2f calcSpawnPosition() const; // calculate object position when spawn
  sf::Vector2f updateVelocity(const float &speed) const; //{speed * direction, 0}
  void adjustSprite(); // scale size of sprite if its size is larger than CELL_SIZE

public:
  // contructor, destructor
  DynamicObject(const sf::Texture &texture, 
      Lane *init_lane = nullptr);
  virtual ~DynamicObject() = default;

  // get, set
  sf::Vector2f getPosition() const;
  void setPosition(const sf::Vector2f &pos);

  sf::Vector2f getVelocity() const;
  void setVelocity(const sf::Vector2f &velo);

  sf::Sprite getSprite() const;
  void setSprite(const sf::Sprite &new_sprite);

  sf::FloatRect getHitBox() const;
  // set hitbox in protected

  sf::Vector2f getHitBoxOffset() const;
  // set hitbox_offset in protected

  bool isActive() const;
  void setActiveState(const bool &state);

  Lane *getLane() const;
  void setLane(Lane *lane);

  int getDirection() const; // 1 = right, -1 = left
  void setDirection(const int &direct); // accepts only 1 or -1; re-flips the sprite accordingly

  float getWidthSprite() const;
  float getHeightSprite() const;

  // Save/Load
  // virtual void serialize(std::ostream& outDev) const; //write state of object to binary file 
  // virtual void deserialize(std::istream& inDev); //read state of object from binary file

  // Other
  // bool isColliding(const sf::FloatRect& other) const; //true if other rectangle intersects this object's hitbox

  bool isOutOfMap();
  float gapToObstacle(const float& obstacle_position_x, const float& obstacle_width) const;
  virtual void render(sf::RenderWindow &window) const; // draw sprite to screen when active
  virtual Constants::DynamicObjectType getType() const = 0; // object type as a DynamicObjectType enum
  virtual void updateState(float delta_time) = 0; // update state for this frame (delta_time: seconds since the previous frame)
};