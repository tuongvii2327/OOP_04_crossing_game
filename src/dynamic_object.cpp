#include "../include/entities/dynamic_object.h"
#include "../include/world/lane.h"

#pragma region PRIVATE

// reposition hitbox from position and offset (hitbox.size stays as set)
void DynamicObject::updateHitbox() 
{
  hitbox.position = position + hitbox_offset;
}

// flip the sprite horizontally when direction == -1, keep it as-is when
// direction == 1
void DynamicObject::applyFacing() 
{
  if (direction == -1) {
    sprite.setScale({-1.f, 1.f});
    sprite.setOrigin({getWidthSprite(), 0.f});
  } else {
    sprite.setScale({1.f, 1.f});
    sprite.setOrigin({0.f, 0.f});
  }
}
#pragma endregion

#pragma region PROTECTED
// set offset, size and recalculate hitbox
void DynamicObject::setHitbox()
{
    setHitboxSize();
    setHitboxOffset();
    updateHitbox();
}

// HITBOX_SIZE_RATIO * sprite size (same for every dynamic object)
void DynamicObject::setHitboxSize()
{
    hitbox.size = sprite.getGlobalBounds().size * Constants::HITBOX_SIZE_RATIO;
}

void DynamicObject::setHitboxOffset()
{
    hitbox_offset = (sprite.getGlobalBounds().size - hitbox.size) / 2.f;
}

// move by a relative offset, then sync sprite and hitbox
void DynamicObject::moveBy(sf::Vector2f offset) 
{
  position += offset;
  sprite.setPosition(position);
  updateHitbox();
}

// pick a random float from the speed range for 'type', scaled by level
// speed = random number in [min_speed, max_speed] * (1 + (level - 1) *
// SPEED_LEVEL_FACTOR)
float DynamicObject::randomSpeed(Constants::DynamicObjectType type) const 
{
  int level = current_lane->getLevel();
  static std::mt19937 rng(std::random_device{}());
  Constants::SpeedRange range = Constants::getSpeedRange(type);
  std::uniform_int_distribution<int> dist(range.min_speed, range.max_speed);
  float base = static_cast<float>(dist(rng));
  float factor = 1.f + static_cast<float>(level - 1) * Constants::SPEED_LEVEL_FACTOR;
  return base * factor;
}

#pragma region CALCULATE FOR VARIANTS
// direction taken from current_lane (1 if no lane attached)
int DynamicObject::calcDirection() const 
{
  return (current_lane != nullptr) ? current_lane->getDirection() : 1;
}

// calculate object position when spawn
sf::Vector2f DynamicObject::calcSpawnPosition() const 
{
  if (current_lane == nullptr)
    return sf::Vector2f(0.f, 0.f);
  sf::Vector2f lane_position = current_lane->getPosition();
  float spawn_y = lane_position.y + (Constants::LANE_HEIGHT - getHeightSprite()) / 2.f;
  float spawn_x = (direction == 1) 
      ? lane_position.x - getWidthSprite()
      : lane_position.x + Constants::LANE_WIDTH;
  return sf::Vector2f(spawn_x, spawn_y);
}

//{speed * direction, 0}
sf::Vector2f DynamicObject::updateVelocity(const float &speed) const 
{
  return sf::Vector2f(speed * static_cast<float>(direction), 0.f);
}

// scale size of sprite if its size is larger than CELL_SIZE
void DynamicObject::adjustSprite() 
{
  if (getWidthSprite() == 0.f || getHeightSprite() == 0.f)
    return;
  if (getWidthSprite() > Constants::LANE_WIDTH || getHeightSprite() > Constants::LANE_HEIGHT) {
    float scaleX = (Constants::LANE_WIDTH - Constants::VISUAL_DISTANCE) / getWidthSprite();
    float scaleY = (Constants::LANE_HEIGHT - Constants::VISUAL_DISTANCE) / getHeightSprite();
    float finalScale = std::min(scaleX, scaleY);
    sprite.setScale({finalScale, finalScale});
  }
}
#pragma endregion
#pragma endregion

#pragma region PUBLIC

#pragma region CONSTRUCTOR, DESTRUCTOR
// Only texture + lane are passed in; direction, position, hitbox are computed from the lane and sprite.
DynamicObject::DynamicObject(const sf::Texture &texture, 
    Lane *init_lane)
    : sprite(texture) 
{
  adjustSprite();
  setLane(init_lane);
  setDirection(calcDirection()); // direction from lane, then flips the sprite when moving left
  setPosition(calcSpawnPosition());
  setHitbox(); // set hitbox_offset automatically here
  setVelocity(sf::Vector2f(0.f, 0.f)); // subclasses set the real velocity once they know their speed
  setActiveState(true);
}

// virtual ~DynamicObject() = default;
#pragma endregion

#pragma region GET, SET
sf::Vector2f DynamicObject::getPosition() const 
{ 
    return position; 
}

void DynamicObject::setPosition(const sf::Vector2f &pos) 
{
  position = pos;
  sprite.setPosition(pos);
  updateHitbox();
}

sf::Vector2f DynamicObject::getVelocity() const 
{ 
    return velocity; 
}

void DynamicObject::setVelocity(const sf::Vector2f &velo) 
{ 
    velocity = velo; 
}

sf::Sprite DynamicObject::getSprite() const 
{ 
    return sprite; 
}

void DynamicObject::setSprite(const sf::Sprite &new_sprite) 
{
  sprite = new_sprite;
  adjustSprite();
}

sf::FloatRect DynamicObject::getHitBox() const 
{ 
    return hitbox; 
}
// set hitbox in protected

sf::Vector2f DynamicObject::getHitBoxOffset() const 
{ 
    return hitbox_offset; 
}

bool DynamicObject::isActive() const 
{ 
    return is_active; 
}

void DynamicObject::setActiveState(const bool &state) 
{ 
    is_active = state; 
}

Lane *DynamicObject::getLane() const 
{ 
    return current_lane; 
}

void DynamicObject::setLane(Lane *lane) 
{ 
    current_lane = lane; 
}

// 1 = right, -1 = left
int DynamicObject::getDirection() const 
{ 
    return direction; 
}

// accepts only 1 or -1; re-flips the sprite accordingly
void DynamicObject::setDirection(const int &direct) 
{
  if (direct == 1 || direct == -1)
    direction = direct;
  applyFacing();
}

float DynamicObject::getWidthSprite() const
{
    return sprite.getGlobalBounds().size.x;
}

float DynamicObject::getHeightSprite() const
{
    return sprite.getGlobalBounds().size.y;
}
#pragma endregion

#pragma region SAVE/LOAD

//// write state of object to binary file
//void DynamicObject::serialize(std::ostream& outDev) const
//{
//    outDev.write(reinterpret_cast<const char*>(&position), sizeof(position));
//    outDev.write(reinterpret_cast<const char*>(&velocity), sizeof(velocity)); 	
//    outDev.write(reinterpret_cast<const char*>(&is_active), sizeof(is_active)); 	
//    outDev.write(reinterpret_cast<const char*>(&direction), sizeof(direction));
//}

////read state of object from binary file
// void DynamicObject::deserialize(std::istream& inDev)
//{
//	inDev.read(reinterpret_cast<char*>(&position), sizeof(position));
//	inDev.read(reinterpret_cast<char*>(&velocity), sizeof(velocity));
//	inDev.read(reinterpret_cast<char*>(&is_active), sizeof(is_active));
//	inDev.read(reinterpret_cast<char*>(&direction), sizeof(direction));
//
//	//restore sprite: re-apply the facing flip, then place it at the loadedposition 	
//    applyFacing();
//    sprite.setPosition(position); 
//    updateHitbox();
//}
#pragma endregion

#pragma region OTHER

// true if other rectangle intersects this object's hitbox
// bool DynamicObject::isColliding(const sf::FloatRect& other) const
//{
//	return hitbox.findIntersection(other).has_value();
// }

bool DynamicObject::isOutOfMap() {
  if (position.x < (Constants::GAME_PLAY_BAR_WIDTH - getWidthSprite()) ||
      position.x > Constants::WINDOW_WIDTH ||
      position.y < - getHeightSprite() ||
      position.y > Constants::WINDOW_HEIGHT) 
  {
    if (isActive())
      setActiveState(false);
    return true;
  }
  return false;
}

float DynamicObject::gapToObstacle(const float& obstacle_position_x, const float& obstacle_width) const
{
    float curr_front_x = (direction == 1)
        ? (position.x + getWidthSprite())
        : (position.x);
    float near_edge = (direction == 1)
        ? (obstacle_position_x)
        : (obstacle_position_x + obstacle_width);
    return direction * (near_edge - curr_front_x) - Constants::VISUAL_DISTANCE;
}

// draw sprite to screen when active
void DynamicObject::render(sf::RenderWindow &window) const 
{
  if (is_active)
    window.draw(sprite);
}

// virtual Constants::DynamicObjectType getType() const = 0;
// virtual void updateState(float delta_time) = 0;
#pragma endregion
#pragma endregion