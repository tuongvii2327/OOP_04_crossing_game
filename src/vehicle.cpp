#include "../include/entities/vehicle.h"
#include "../include/world/lane.h"

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
  float v_light = base_speed;
  float v_vehicle = base_speed;
  Traffic *nearest_traffic = current_lane->getNearestTrafficAhead(*this);
  if (nearest_traffic)
  {
      float gap = gapToObstacle(nearest_traffic->getPosition().x, nearest_traffic->getWidthSprite());
      float ratio = std::clamp(gap / safe_distance, 0.f, 1.f);
      v_light = base_speed * ratio;
  }
  Vehicle *nearest_vehicle = current_lane->getNearestVehicleAhead(*this);
  if (nearest_vehicle)
  {
      float gap = gapToObstacle(nearest_vehicle->getPosition().x, nearest_vehicle->getWidthSprite());
      float ratio = std::clamp(gap / safe_distance, 0.f, 1.f);
      float nearest_vehicle_speed = nearest_vehicle->getVelocity().x * direction;
      v_vehicle = nearest_vehicle_speed + ratio * (base_speed - nearest_vehicle_speed);
  }
  float v_target = std::min(std::min(base_speed, v_light), v_vehicle);
  float acceleration = std::pow(base_speed, 2.0) / (safe_distance);
  if (v_target > velocity.x * direction)
  {
      v_target = std::min(v_target, velocity.x * direction + acceleration * delta_time);
  }
  setVelocity({ v_target * direction, 0.0f });
  moveBy(velocity * delta_time);
}
#pragma endregion
#pragma endregion