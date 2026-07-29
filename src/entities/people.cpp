#include "entities/people.h"
#include "utils/audio_system.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"
#include "utils/serialization.h"
#include <algorithm>

People::People(Constants::ObjectColor color)
    : DynamicObject(Constants::DynamicObjectType::People, color, nullptr),
      is_dead(false), step_size(static_cast<float>(Constants::LANE_HEIGHT)),
      people_color(color), current_dir(Direction::Up),
      current_anim_state(AnimState::Idle), anim_timer(0.f),
      anim_duration(0.15f), is_animating(false) {
  sprite.setOrigin({0.f, 0.f});

  updateSpriteRect();

  float scale_x = 52.f / static_cast<float>(FRAME_W);
  float scale_y = 62.f / static_cast<float>(FRAME_H);
  sprite.setScale({scale_x, scale_y});

  hitbox.size = {52.f * Constants::HITBOX_SIZE_RATIO,
                 62.f * Constants::HITBOX_SIZE_RATIO};
  hitbox_offset = {(52.f - hitbox.size.x) / 2.f, (62.f - hitbox.size.y) / 2.f};

  resetPosition();
}

People::People(float x, float y, Constants::ObjectColor color) : People(color) {
  setPosition(sf::Vector2f(x, y));
}

Constants::DynamicObjectType People::getType() const {
  return Constants::DynamicObjectType::People;
}

Constants::ObjectColor People::getColor() const { return people_color; }

void People::updateSpriteRect() {
  int col = static_cast<int>(current_anim_state);
  int row = static_cast<int>(current_dir);
  int left = col * FRAME_W;
  int top = row * FRAME_H;
  sprite.setTextureRect(
      sf::IntRect(sf::Vector2i(left, top), sf::Vector2i(FRAME_W, FRAME_H)));
}

void People::updateState(float delta_time) {
  if (is_dead || !is_active)
    return;

  if (is_animating) {
    anim_timer += delta_time;
    float frame_time = anim_duration / 3.0f;

    // Smoothstep: t = 3t^2 - 2t^3, cho cảm giác "spring" tự nhiên
    float raw_t = std::min(anim_timer / anim_duration, 1.0f);
    float t = raw_t * raw_t * (3.0f - 2.0f * raw_t);
    position =
        start_lerp_position + t * (target_position - start_lerp_position);
    sprite.setPosition(position);
    hitbox.position = position + hitbox_offset; // hitbox bám sát sprite lerp

    if (anim_timer >= anim_duration) {
      // Snap chính xác về target để loại bỏ sai số float
      position = target_position;
      sprite.setPosition(position);
      hitbox.position = target_position +
                        hitbox_offset; // snap hitbox chính xác khi xong anim
      is_animating = false;
      current_anim_state = AnimState::Idle;
    } else if (anim_timer >= frame_time * 2) {
      current_anim_state = AnimState::Land;
    } else if (anim_timer >= frame_time) {
      current_anim_state = AnimState::Jump;
    } else {
      current_anim_state = AnimState::Prep;
    }

    updateSpriteRect();
  }
}

bool People::isColliding(const sf::FloatRect &other_hitbox) const {
  if (is_dead || !is_active)
    return false;
  return hitbox.findIntersection(other_hitbox).has_value();
}

bool People::tryToMove(float dx, float dy, float camera_top_y, const Map &map) {
  if (is_dead)
    return false;

  // Block input khi đang trong animation để đảm bảo mỗi bước nhảy hoàn chỉnh
  if (is_animating)
    return false;

  // Kiểm tra biên dựa trên target_position (vị trí logic hiện tại)
  float future_left = target_position.x + hitbox_offset.x + dx;
  float future_right = future_left + hitbox.size.x;
  float future_top = target_position.y + hitbox_offset.y + dy;
  float future_bottom = future_top + hitbox.size.y;

  float left_bound =
      static_cast<float>(Constants::GAME_PLAY_BAR_WIDTH); // left (140)
  float right_bound =
      static_cast<float>(Constants::WINDOW_WIDTH); // right (1400)
  float cam_bottom =
      camera_top_y + static_cast<float>(Constants::WINDOW_HEIGHT);

  // Kiểm tra biên màn hình
  if (future_left < left_bound || future_right > right_bound ||
      future_top < camera_top_y || future_bottom > cam_bottom) {
    return false;
  }

  // Kiểm tra va chạm với ô blocked trong map
  sf::FloatRect future_hitbox_rect({future_left, future_top},
                                   {hitbox.size.x, hitbox.size.y});
  if (map.checkBlockedCollision(future_hitbox_rect))
    return false;

  // Lưu điểm xuất phát (vị trí visual hiện tại) và cập nhật điểm đích logic
  // Không gọi moveBy() vì sprite/hitbox được điều khiển hoàn toàn bởi lerp
  // trong updateState()
  start_lerp_position = position;
  target_position = target_position + sf::Vector2f(dx, dy);

  Audio().playSFX(AudioSystem::SFX::Jump);

  is_animating = true;
  anim_timer = 0.f;
  current_anim_state = AnimState::Prep;
  updateSpriteRect();

  return true;
}

void People::die() { is_dead = true; }

bool People::isDead() const { return is_dead; }

bool People::isAnimating() const { return is_animating; }

sf::Vector2f People::getStartPosition(int init_cell, int init_lane) const {
  float start_x =
      static_cast<float>(Constants::WINDOW_WIDTH) -
      static_cast<float>(Constants::NUMBER_OF_CELLS - init_cell + 1) *
          static_cast<float>(Constants::CELL_SIZE) +
      (static_cast<float>(Constants::CELL_SIZE) - 52.f) / 2.f;
  float start_y =
      (static_cast<float>(Constants::WINDOW_HEIGHT) -
       static_cast<float>(Constants::NUMBER_OF_LANES - init_lane + 1) *
           static_cast<float>(Constants::LANE_HEIGHT)) +
      (static_cast<float>(Constants::LANE_HEIGHT) - 62.f) / 2.f;
  return sf::Vector2f(start_x, start_y);
}

void People::resetPosition() {
  position = getStartPosition();
  // Đồng bộ target & start để không có Lerp ghost khi reset
  target_position = position;
  start_lerp_position = position;
  sprite.setPosition(position);
  hitbox.position = position + hitbox_offset;

  is_dead = false;
  is_active = true;

  current_anim_state = AnimState::Idle;
  current_dir = Direction::Up;
  is_animating = false;
  updateSpriteRect();
}

sf::FloatRect People::getBounds() const { return sprite.getGlobalBounds(); }

bool People::handleEvent(const sf::Event &event, float camera_top_y,
                         const Map &map) {
  if (is_dead || !is_active)
    return false;

  const auto *key = event.getIf<sf::Event::KeyPressed>();
  if (!key)
    return false;

  if (key->code == sf::Keyboard::Key::W || key->code == sf::Keyboard::Key::Up) {
    current_dir = Direction::Up;
    updateSpriteRect();
    return tryToMove(0.f, -step_size, camera_top_y, map);
  } else if (key->code == sf::Keyboard::Key::S ||
             key->code == sf::Keyboard::Key::Down) {
    current_dir = Direction::Down;
    updateSpriteRect();
    return tryToMove(0.f, step_size, camera_top_y, map);
  } else if (key->code == sf::Keyboard::Key::A ||
             key->code == sf::Keyboard::Key::Left) {
    current_dir = Direction::Left;
    updateSpriteRect();
    return tryToMove(-step_size, 0.f, camera_top_y, map);
  } else if (key->code == sf::Keyboard::Key::D ||
             key->code == sf::Keyboard::Key::Right) {
    current_dir = Direction::Right;
    updateSpriteRect();
    return tryToMove(step_size, 0.f, camera_top_y, map);
  }

  return false;
}

void People::serialize(std::ostream &os) const {
  Serialization::write(os, is_dead);
  Serialization::write(os, position.x);
  Serialization::write(os, position.y);
  Serialization::write(os, step_size);
  Serialization::write(os, is_active);

  int dir_int = static_cast<int>(current_dir);
  Serialization::write(os, dir_int);
}

void People::deserialize(std::istream &is) {
  Serialization::read(is, is_dead);
  Serialization::read(is, position.x);
  Serialization::read(is, position.y);
  Serialization::read(is, step_size);
  Serialization::read(is, is_active);

  int dir_int = 0;
  Serialization::read(is, dir_int);
  current_dir = static_cast<Direction>(dir_int);

  is_dead = false;
  is_active = true;
  is_animating = false;
  current_anim_state = AnimState::Idle;

  updateSpriteRect();
  sprite.setPosition(position);
  hitbox.position = position + hitbox_offset;
  // Đồng bộ target & start sau khi load để không có Lerp ghost
  target_position = position;
  start_lerp_position = position;
}