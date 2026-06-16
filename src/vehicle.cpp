#include "../include/entities/vehicle.h"
#include "../include/world/lane.h"

// Convert a gap into the fraction of base_speed to use: clamp(gap /
// safe_distance, 0, 1). gap >= safe_distance -> full speed, gap -> 0 -> stop.
// Used for both the light and the vehicle ahead.
static float speedRatio(float gap, float safe_distance) 
{
  if (safe_distance <= 0.f)
    return 1.f;
  float ratio = gap / safe_distance;
  if (ratio < 0.f)
    return 0.f;
  if (ratio > 1.f)
    return 1.f;
  return ratio;
}

#pragma region PROTECTED

// randomSpeed for type, scaled by lane level
float Vehicle::calcBaseSpeed(Constants::DynamicObjectType type) const 
{
  return randomSpeed(type);
}

// temp formula: braking/following distance from base_speed
float Vehicle::calcSafeDistance() const 
{
  return base_speed * Constants::SAFE_DISTANCE_FACTOR;
}
#pragma endregion

#pragma region PUBLIC
#pragma region CONSTRUCTOR, DESTRUCTOR
Vehicle::Vehicle(const sf::Texture &texture, 
    Constants::DynamicObjectType type,
    Lane *init_lane)
    : DynamicObject(texture, init_lane) 
{
  setBaseSpeed(calcBaseSpeed(type)); // set safe_distance automatically here
  setVelocity(updateVelocity(base_speed)); // direction is already set by DynamicObject
  setMotionState(Constants::MotionState::Cruising);
  setLightDecision(Constants::LightDecision::None);
}
#pragma endregion

#pragma region GET, SET
float Vehicle::getBaseSpeed() const 
{ 
    return base_speed; 
}

void Vehicle::setBaseSpeed(const float &newSpeed) 
{
  base_speed = newSpeed;
  safe_distance = calcSafeDistance();
}

float Vehicle::getSafeDistance() const 
{ 
    return safe_distance; 
}

Constants::MotionState Vehicle::getMotionState() const 
{ 
    return motion_state; 
}

void Vehicle::setMotionState(const Constants::MotionState &state) 
{
  motion_state = state;
  if (state == Constants::MotionState::Cruising)
    setLightDecision(Constants::LightDecision::None); // forget the old light decision
}

Constants::LightDecision Vehicle::getLightDecision() const 
{
  return light_decision;
}

void Vehicle::setLightDecision(const Constants::LightDecision &decision) 
{
  light_decision = decision;
}
#pragma endregion

#pragma region SAVE/LOAD
////write state of object to binary file
// void Vehicle::serialize(std::ostream& outDev) const
//{
//	DynamicObject::serialize(outDev); //writes position, velocity,
//is_active, direction 	outDev.write(reinterpret_cast<const char*>(&base_speed),
//sizeof(base_speed));
//	//motion state is saved too so loading restores the vehicle exactly as
//it was 	outDev.write(reinterpret_cast<const char*>(&motion_state),
//sizeof(motion_state)); 	outDev.write(reinterpret_cast<const
//char*>(&light_decision), sizeof(light_decision));
//	outDev.write(reinterpret_cast<const char*>(&launch_distance),
//sizeof(launch_distance));
// }

////read state of object from binary file
// void Vehicle::deserialize(std::istream& inDev)
//{
//	DynamicObject::deserialize(inDev); //reads direction and re-applies the
//sprite flip 	inDev.read(reinterpret_cast<char*>(&base_speed),
//sizeof(base_speed)); 	inDev.read(reinterpret_cast<char*>(&motion_state),
//sizeof(motion_state)); 	inDev.read(reinterpret_cast<char*>(&light_decision),
//sizeof(light_decision)); 	inDev.read(reinterpret_cast<char*>(&launch_distance),
//sizeof(launch_distance));

//	//safe_distance is derived from base_speed and not saved, recompute it
//from the loaded base_speed 	safe_distance = calcSafeDistance();
//}
#pragma endregion

#pragma region OTHER

// void Vehicle::playHornSound()
//{
//	horn_sound.play();
// }

void Vehicle::updateState(float delta_time) 
{
  if (isOutOfMap() || !is_active)
    return;
  Traffic *nearest_traffic = current_lane->getNearestTrafficAhead(*this);
  Vehicle *nearest_vehicle = current_lane->getNearestVehicleAhead(*this);
  if (nearest_traffic || nearest_vehicle) 
  {
    bool traffic_is_nearest = 1;
    if (nearest_traffic && nearest_vehicle &&
        nearest_traffic->getPosition().x > nearest_vehicle->getPosition().x) 
    {
      traffic_is_nearest = 0;
    } 
    else if (!nearest_traffic && nearest_vehicle) 
    {
      traffic_is_nearest = 0;
    }

    float vehicle_width = sprite.getLocalBounds().size.x;
    if (traffic_is_nearest) 
    {
      float traffic_width = nearest_traffic->getSprite().getLocalBounds().size.x;
      float gap = (direction == 1)
          ? (nearest_traffic->getPosition().x - position.x - Constants::VISUAL_DISTANCE - vehicle_width)
          : - (position.x - nearest_traffic->getPosition().x - Constants::VISUAL_DISTANCE - traffic_width);
      float speed_ratio = gap / safe_distance;
      setVelocity(velocity * speed_ratio);
    } 
    else 
    {
      float nearest_vehicle_width = nearest_vehicle->getSprite().getLocalBounds().size.x;
      sf::Vector2f delta_velocity = velocity - nearest_vehicle->getVelocity();
      float gap = (direction == 1)
          ? (nearest_vehicle->getPosition().x - position.x - Constants::VISUAL_DISTANCE - vehicle_width)
          : (position.x - nearest_vehicle->getPosition().x - Constants::VISUAL_DISTANCE - nearest_vehicle_width);
      sf::Vector2f final_velocity = velocity - (1 - (gap / safe_distance)) * delta_velocity;
      setVelocity(final_velocity);
    }
  }
  moveBy(velocity * delta_time);
}
#pragma endregion
#pragma endregion