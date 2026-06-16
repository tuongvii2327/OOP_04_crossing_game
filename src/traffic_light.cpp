#include "../include/entities/traffic_light.h"
#include "../include/world/lane.h" //full definition needed in .cpp

#pragma region PROTECTED
void Traffic::adjustSprite() 
{
  float currentWidth = sprite.getLocalBounds().size.x;
  float currentHeight = sprite.getLocalBounds().size.y;
  if (currentWidth == 0.f || currentHeight == 0.f)
    return;
  if (currentWidth > Constants::TRAFFIC_WIDTH || currentHeight > Constants::TRAFFIC_HEIGHT) 
  {
    float scaleX = Constants::TRAFFIC_WIDTH / currentWidth;
    float scaleY = Constants::TRAFFIC_HEIGHT / currentHeight;
    float finalScale = std::min(scaleX, scaleY);
    sprite.setScale({finalScale, finalScale});
  }
}

float Traffic::randomDuration(Constants::TrafficLightColor color) const 
{
  static std::mt19937 rng(std::random_device{}());
  Constants::DurationRange range = Constants::getDurationRange(color);
  std::uniform_int_distribution<int> dist(range.min_duration, range.max_duration);
  float base = static_cast<float>(dist(rng));
  return base;
}
#pragma endregion

#pragma region PUBLIC
Traffic::Traffic(const sf::Texture &texture, Lane *init_lane,
                 const sf::Vector2f &init_postition,
                 const Constants::TrafficLightColor &init_color,
                 const float &init_timer)
    : sprite(texture) 
{
  adjustSprite();
  setPosition(init_postition);
  setLane(init_lane);
  setColor(init_color);
  setTimer(init_timer); 
  setGreenDuration(randomDuration(Constants::TrafficLightColor::Green));
  setRedDuration(randomDuration(Constants::TrafficLightColor::Red));
}

sf::Sprite Traffic::getSprite() const 
{ 
    return sprite; 
}

void Traffic::setSprite(const sf::Sprite &new_sprite) 
{
  sprite = new_sprite;
  adjustSprite();
}

sf::Vector2f Traffic::getPosition() const 
{ 
    return position; 
}

void Traffic::setPosition(const sf::Vector2f &pos) 
{
  position = pos;
  sprite.setPosition(pos);
}

Lane *Traffic::getLane() const 
{
    return current_lane; 
}

void Traffic::setLane(Lane *lane) 
{ 
    current_lane = lane;
}

Constants::TrafficLightColor Traffic::getColor() const 
{
    return color; 
}

void Traffic::setColor(const Constants::TrafficLightColor &new_color) 
{
  color = new_color;
  setTimer(0.0f);
}

float Traffic::getTimer() const 
{ 
    return timer; 
}

void Traffic::setTimer(const float &new_timer) 
{ 
    timer = new_timer; 
}

float Traffic::getGreenDuration() const 
{ 
    return green_duration; 
}

void Traffic::setGreenDuration(const float &duration) 
{
  green_duration = duration;
}

float Traffic::getRedDuration() const 
{ 
    return red_duration; 
}

void Traffic::setRedDuration(const float &duration) 
{ 
    red_duration = duration; 
}

void Traffic::updateState(float delta_time) 
{
  if (color == Constants::TrafficLightColor::Green && timer >= green_duration) 
  {
    setColor(Constants::TrafficLightColor::Yellow);
    return;
  }
  if (color == Constants::TrafficLightColor::Yellow && timer >= Constants::YELLOW_DURATION) 
  {
    setColor(Constants::TrafficLightColor::Red);
    return;
  }
  if (color == Constants::TrafficLightColor::Red && timer >= red_duration) 
  {
    setColor(Constants::TrafficLightColor::Green);
    return;
  }
  timer += delta_time;
}

void Traffic::render(sf::RenderWindow &window) const 
{ 
    window.draw(sprite); 
}

#pragma endregion