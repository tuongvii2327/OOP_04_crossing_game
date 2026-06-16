#include "../include/world/lane.h"
// Full entity definitions are needed here (forward-declared in lane.h)
#include "../include/entities/car.h"
#include "../include/entities/dynamic_object.h"
#include "../include/entities/traffic_light.h"
#include "../include/entities/truck.h"
#include "../include/entities/vehicle.h"


#pragma region PROTECTED
void Lane::adjustSprite() 
{
  float currentWidth = sprite.getLocalBounds().size.x;
  float currentHeight = sprite.getLocalBounds().size.y;
  if (currentWidth == 0.f || currentHeight == 0.f)
    return;
  if (currentWidth > Constants::LANE_WIDTH || currentHeight > Constants::LANE_HEIGHT) 
  {
    float scaleX = Constants::LANE_WIDTH / currentWidth;
    float scaleY = Constants::LANE_HEIGHT / currentHeight;
    sprite.setScale({scaleX, scaleY});
  }
}

#pragma endregion

#pragma region PUBLIC
Lane::Lane(const sf::Texture &texture, 
    const sf::Vector2f &init_position,
    const int &init_level, 
    const int &init_direction)
    : sprite(texture) 
{
  setPosition(init_position);
  setLevel(init_level);
  setDirection(init_direction);
}

Lane::~Lane() 
{
  for (auto obj : list_obj) 
  {
    delete obj;
    obj = nullptr;
  }
  for (auto obj : list_traffic) 
  {
    delete obj;
    obj = nullptr;
  }
}

sf::Sprite Lane::getSprite() const 
{ 
    return sprite; 
}

void Lane::setSprite(const sf::Sprite &new_sprite) 
{
  sprite = new_sprite;
  adjustSprite();
}

sf::Vector2f Lane::getPosition() const 
{ 
    return position; 
}

void Lane::setPosition(const sf::Vector2f &pos) 
{
  position = pos;
  sprite.setPosition(pos);
}

int Lane::getLevel() const 
{ 
    return level; 
}

void Lane::setLevel(const int &new_level) 
{ 
    level = new_level; 
}

int Lane::getDirection() const 
{ 
    return direction; 
}

void Lane::setDirection(const int &direct) 
{
  if (direct == 1 || direct == -1)
    direction = direct;
}

void Lane::addObject(DynamicObject *obj) 
{ 
    list_obj.push_back(obj); 
}

void Lane::addTraffic(Traffic *t) 
{ 
    list_traffic.push_back(t); 
}

Traffic *Lane::getNearestTrafficAhead(const Vehicle &vehicle) 
{
  std::sort(list_traffic.begin(), list_traffic.end(),
            [](const Traffic *a, const Traffic *b) 
      {
          return a->getPosition().x < b->getPosition().x;
      });
  float vehicle_width = vehicle.getSprite().getLocalBounds().size.x;
  if (direction == 1) 
  {
    float relative_position = vehicle.getPosition().x + vehicle_width + Constants::VISUAL_DISTANCE;
    std::pair<float, float> range = { relative_position, relative_position + vehicle.getSafeDistance()};

    auto it = std::lower_bound(list_traffic.begin(), list_traffic.end(), range.first,
        [](const Traffic *t, float val)
        { 
            return t->getPosition().x < val; 
        });

    while (it != list_traffic.end() && (*it)->getPosition().x <= range.second) 
    {
      Traffic *current = *it;
      if (current->getColor() == Constants::TrafficLightColor::Red) 
      {
        return current;
      }
      if (current->getColor() == Constants::TrafficLightColor::Yellow) 
      {
        float position_after_yellow = (Constants::YELLOW_DURATION - current->getTimer()) * vehicle.getVelocity().x + vehicle.getPosition().x;
        float target_position = current->getPosition().x + Constants::TRAFFIC_WIDTH + Constants::VISUAL_DISTANCE;
        if (position_after_yellow < target_position)
          return current;
      }
      ++it;
    }
  } 
  else 
  {
    float relative_position = vehicle.getPosition().x - Constants::VISUAL_DISTANCE - Constants::TRAFFIC_WIDTH;
    std::pair<float, float> range = {relative_position - vehicle.getSafeDistance(), relative_position};

    auto it = std::lower_bound(list_traffic.rbegin(), list_traffic.rend(), range.second,
        [](const Traffic *t, float val) 
        { 
            return t->getPosition().x > val; 
        });

    while (it != list_traffic.rend() && (*it)->getPosition().x >= range.first) 
    {
      Traffic *current = *it;

      if (current->getColor() == Constants::TrafficLightColor::Red) 
      {
        return current;
      }
      if (current->getColor() == Constants::TrafficLightColor::Yellow) 
      {
        float position_after_yellow = (Constants::YELLOW_DURATION - current->getTimer()) * vehicle.getVelocity().x + vehicle.getPosition().x;
        float target_position = current->getPosition().x - Constants::VISUAL_DISTANCE;
        if (position_after_yellow > target_position)
          return current;
      }
      ++it;
    }
  }
  return nullptr;
}

Vehicle *Lane::getNearestVehicleAhead(const Vehicle &vehicle) 
{
  std::sort(list_obj.begin(), list_obj.end(),
      [](const DynamicObject *a, const DynamicObject *b) 
      {
          return a->getPosition().x < b->getPosition().x;
      });
  float vehicle_width = vehicle.getSprite().getLocalBounds().size.x;
  if (direction == 1) 
  {
    float relative_position = vehicle.getPosition().x + vehicle_width + Constants::VISUAL_DISTANCE;
    std::pair<float, float> range = {relative_position, relative_position + vehicle.getSafeDistance()};

    auto it = std::lower_bound(list_obj.begin(), list_obj.end(), range.first,
        [](const DynamicObject *v, float val) 
        {
            return v->getPosition().x < val;
        });

    while (it != list_obj.end() && (*it)->getPosition().x <= range.second) 
    {
      Vehicle *current = dynamic_cast<Vehicle *>(*it);
      if (current->getVelocity().x < vehicle.getVelocity().x)
        return current;
      ++it;
    }
  } 
  else 
  {
    float relative_position = vehicle.getPosition().x - Constants::VISUAL_DISTANCE;
    std::pair<float, float> range = {relative_position - vehicle.getSafeDistance(), relative_position};

    auto it = std::lower_bound(list_obj.rbegin(), list_obj.rend(), range.second,
        [](const DynamicObject *v, float val) 
        {
            return v->getPosition().x > val;
        });

    while (it != list_obj.rend() && 
        (*it)->getPosition().x >= (range.first - (*it)->getSprite().getLocalBounds().size.x)) 
    {
      Vehicle *current = dynamic_cast<Vehicle *>(*it);
      if (current->getVelocity().x > vehicle.getVelocity().x)
        return current;
      ++it;
    }
  }
  return nullptr;
}

void Lane::render(sf::RenderWindow &window) const 
{ 
    window.draw(sprite); 
}

#pragma endregion
