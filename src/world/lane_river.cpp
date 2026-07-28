#include "world/lane_river.h"
#include "utils/resource_manager.h"
#include "entities/wood.h"

#include <limits>

LaneRiver::LaneRiver(const sf::Vector2f &init_position, const int &init_level,
                     const int &init_direction)
    : Lane(init_position, init_level, init_direction,
           Constants::LaneType::River) {
  seedObjects();
}

LaneRiver::~LaneRiver() = default;

Constants::LaneType LaneRiver::getType() const {
  return Constants::LaneType::River;
}

// populate list_dynamic with initial Wood objects, spread across the lane
void LaneRiver::seedObjects() {
  // River lanes always have enough Wood for the player to cross
  int min_spawn = 2;
  int max_spawn = 3;

  if (level >= 10) {
    min_spawn = 1;
    max_spawn = 2;
  } else if (level >= 3) {
    min_spawn = 2;
    max_spawn = 3;
  } else {
    min_spawn = 2;
    max_spawn = 3;
  }
  int count = std::min(randomRange(min_spawn, max_spawn),
                       static_cast<int>(Constants::MAX_OBJECT_PER_LANE));

  const float min_gap = Constants::CELL_SIZE * 1.5f;

  for (int i = 0; i < count; ++i) {
    Constants::ObjectColor color = Constants::ObjectColor::None;

    auto obj = std::make_unique<Wood>(color, this);

    float obj_width = obj->getWidthSprite();
    float lane_left = position.x,
          lane_right = position.x + Constants::LANE_WIDTH - obj_width;
    int range = lane_right - lane_left;
    if (range < 0) {
      range = 0;
    }
    float x = lane_left;
    bool placed = false;
    for (int tryy = 0; tryy < 10; tryy++) {
      x = lane_left + randomRange(0, range);
      bool overlap = false;
      for (auto &other : list_dynamic) {
        float other_x = other->getPosition().x;
        float other_width = other->getWidthSprite();
        if (x < other_x + other_width + min_gap &&
            x + other_width + min_gap > other_x) {
          overlap = true;
          break;
        }
      }
      if (!overlap) {
        placed = true;
        break;
      }
    }

    obj->setPosition({x, 0});
    list_dynamic.push_back(std::move(obj));
  }

  std::sort(list_dynamic.begin(), list_dynamic.end(),
            [this](const std::unique_ptr<DynamicObject> &a,
                   const std::unique_ptr<DynamicObject> &b) {
              return a->getPosition().x * direction >
                     b->getPosition().x * direction;
            });
  num_active_object = static_cast<int>(list_dynamic.size());
}

// probabilistically spawn (or reuse a recycled inactive Wood) once the lane
// has room and the most-recently-entered object has cleared the entry edge
void LaneRiver::spawnObject() {
  int max_allowed = 3;
  if (level >= 10) {
    max_allowed = 2;
  } else if (level >= 3) {
    max_allowed = 3;
  }

  if (num_active_object >= max_allowed)
    return;
  if (list_dynamic.empty())
    return;

  int spawn_rate_chance = (level >= 6) ? 1 : 2;
  if (randomRange(0, spawn_rate_chance) != 0)
    return;

  DynamicObject *last_obj = list_dynamic.back().get();
  float lane_left = position.x;
  float lane_right = position.x + Constants::LANE_WIDTH;
  float entered_distance = (direction == 1)
                               ? (last_obj->getPosition().x - lane_left)
                               : (lane_right - (last_obj->getPosition().x +
                                                last_obj->getWidthSprite()));
  if (entered_distance < static_cast<float>(Constants::CELL_SIZE))
    return;

  Constants::ObjectColor exp_color = Constants::ObjectColor::None;
  bool is_spawn = false;

  // reuse an inactive Wood object sitting in the exit prefix
  for (auto o = list_dynamic.begin(); o != list_dynamic.end() && !(*o)->isActive();) {
    if ((*o)->getType() == Constants::DynamicObjectType::Wood) {
      Wood *target = dynamic_cast<Wood *>(o->get());
      if (target) {
        target->setActiveState(true);
        target->setPosition(target->calcSpawnPosition());
        
        float new_speed = target->randomSpeed(Constants::DynamicObjectType::Wood);
        target->setBaseSpeed(target->randomSpeed(Constants::DynamicObjectType::Wood));
        target->setVelocity({new_speed * direction, 0.f});
        
        std::unique_ptr<DynamicObject> moved = std::move(*o);
        o = list_dynamic.erase(o);
        list_dynamic.push_back(std::move(moved));
        
        is_spawn = true;
      }
      break;
    }
    ++o;
  }
  if (!is_spawn) {
    list_dynamic.push_back(std::make_unique<Wood>(exp_color, this));
  }
}

// Helper: returns true if the player's center X is within the wood's horizontal
// bounds. This means the player can stand on wood only if they haven't stepped
// more than half off it.
static bool isPlayerOnWood(const sf::FloatRect &player_hitbox,
                           const DynamicObject *wood) {
  float player_center_x = player_hitbox.position.x + player_hitbox.size.x / 2.f;
  float wood_left = wood->getHitBox().position.x;
  float wood_right = wood_left + wood->getHitBox().size.x;
  return player_center_x >= wood_left && player_center_x <= wood_right;
}

// River is lethal: if the player's hitbox overlaps this lane but is NOT
// standing on any active Wood, they drown.
bool LaneRiver::checkLethalCollision(const sf::FloatRect &player_hitbox) const {
  // 1. Check if player is actually within this lane's vertical bounds
  float lane_top = position.y;
  float lane_bottom = position.y + Constants::LANE_HEIGHT;
  float player_center_y = player_hitbox.position.y + player_hitbox.size.y / 2.f;

  if (player_center_y < lane_top || player_center_y > lane_bottom)
    return false; // player is not on this lane

  // 2. Check if player's center X is within any active Wood's bounds
  for (const auto &obj : list_dynamic) {
    if (!obj->isActive())
      continue;
    if (obj->getType() != Constants::DynamicObjectType::Wood)
      continue;

    if (isPlayerOnWood(player_hitbox, obj.get()))
      return false; // player is on a Wood, safe!
  }

  // 3. Player is on this river lane but not on any Wood -> drown
  return true;
}

// Returns the velocity of the Wood the player is currently standing on,
// or {0,0} if the player is not on any Wood in this lane.
sf::Vector2f
LaneRiver::getWoodVelocityAt(const sf::FloatRect &player_hitbox) const {
  float lane_top = position.y;
  float lane_bottom = position.y + Constants::LANE_HEIGHT;
  float player_y = player_hitbox.position.y;

  if (player_y < lane_top || player_y > lane_bottom)
    return {0.f, 0.f};

  for (const auto &obj : list_dynamic) {
    if (!obj->isActive())
      continue;
    if (obj->getType() != Constants::DynamicObjectType::Wood)
      continue;

    if (isPlayerOnWood(player_hitbox, obj.get()))
      return obj->getVelocity();
  }

  return {0.f, 0.f};
}

// the wood immediately ahead of wood in list_dynamic, if it is slower
Wood *LaneRiver::getNearestWoodAhead(const Wood &wood) {
  for (auto it = list_dynamic.begin(); it != list_dynamic.end(); ++it) {
    if (it->get() != &wood)
      continue;
    if (it == list_dynamic.begin())
      return nullptr;
    auto ahead_it = std::prev(it);
    if (!(*ahead_it)->isActive())
      return nullptr;
    Wood *ahead = dynamic_cast<Wood *>(ahead_it->get());
    if (ahead &&
        ahead->getVelocity().x * direction < wood.getVelocity().x * direction)
      return ahead;
    return nullptr;
  }
  return nullptr;
}

void LaneRiver::updateState(float delta_time)
{
    Lane::updateState(delta_time);
    texture_variant++;
    setSprite(Resource().getLaneTexture(Constants::LaneType::River, texture_variant));
}
