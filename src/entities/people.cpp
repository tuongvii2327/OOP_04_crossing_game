#include "entities/people.h"
#include "utils/audio_system.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"
#include "utils/serialization.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

People::People(Constants::ObjectColor color)
    : DynamicObject(Constants::DynamicObjectType::People, color, nullptr),
      is_dead(false),
      step_size(static_cast<float>(Constants::LANE_HEIGHT)),
      people_color(color),
      current_dir(Direction::Up),
      current_anim_state(AnimState::Idle),
      anim_timer(0.f),
      anim_duration(0.3f),
      is_animating(false)
{
    sprite.setOrigin({0.f, 0.f});

    base_scale_x_ = 52.f / static_cast<float>(FRAME_W);
    base_scale_y_ = 62.f / static_cast<float>(FRAME_H);
    sprite.setScale(sf::Vector2f(base_scale_x_, base_scale_y_));

    hitbox.size = {52.f * Constants::HITBOX_SIZE_RATIO,
                   62.f * Constants::HITBOX_SIZE_RATIO};
    hitbox_offset = {(52.f - hitbox.size.x) / 2.f,
                     (62.f - hitbox.size.y) / 2.f};

    resetPosition();
}

People::People(float x, float y, Constants::ObjectColor color) : People(color)
{
    setPosition(sf::Vector2f(x, y));
}

Constants::DynamicObjectType People::getType() const
{
    return Constants::DynamicObjectType::People;
}

Constants::ObjectColor People::getColor() const { return people_color; }

void People::updateSpriteRect()
{
    int col = static_cast<int>(current_anim_state);
    int row = static_cast<int>(current_dir);
    int left = col * FRAME_W;
    int top = row * FRAME_H;
    sprite.setTextureRect(
        sf::IntRect(sf::Vector2i(left, top), sf::Vector2i(FRAME_W, FRAME_H)));
}

void People::updateState(float delta_time)
{
    if (is_dead || !is_active)
        return;

    if (is_animating)
    {
        anim_timer += delta_time;
        float progress = std::min(anim_timer / anim_duration, 1.0f);

        // Keep hitbox at start position during jump (no mid-air collision)
        hitbox.position = jump_start_pos + hitbox_offset;

        // --- Visual sprite interpolation ---
        float vis_x = jump_start_pos.x + (jump_end_pos.x - jump_start_pos.x) * progress;
        float arc = -JUMP_ARC_HEIGHT * std::sin(static_cast<float>(M_PI) * progress);
        float vis_y = jump_start_pos.y + (jump_end_pos.y - jump_start_pos.y) * progress + arc;
        sprite.setPosition(sf::Vector2f(vis_x, vis_y));

        // Không xoay người trong lúc nhảy: nhân vật nhảy thẳng, sprite đổi
        // hướng ngay khi bấm phím, tránh hiệu ứng lộn vòng khi đổi hướng
        sprite.setOrigin({0.f, 0.f});
        sprite.setRotation(sf::degrees(0.f));

        // --- Animation frame selection + squash/stretch scale ---
        if (progress < 0.2f)
        {
            current_anim_state = AnimState::Prep;
            float t = progress / 0.2f;
            float squash = std::sin(t * static_cast<float>(M_PI) * 0.5f);
            sprite.setScale(sf::Vector2f(
                base_scale_x_ * (1.f + 0.15f * squash),
                base_scale_y_ * (1.f - 0.18f * squash)));
        }
        else if (progress < 0.65f)
        {
            current_anim_state = AnimState::Jump;
            float t = (progress - 0.2f) / 0.45f;
            float stretch = std::sin(t * static_cast<float>(M_PI));
            sprite.setScale(sf::Vector2f(
                base_scale_x_ * (1.f - 0.10f * stretch),
                base_scale_y_ * (1.f + 0.12f * stretch)));
        }
        else
        {
            float t = (progress - 0.65f) / 0.35f;
            if (t < 0.5f)
            {
                current_anim_state = AnimState::Land;
                float s = t / 0.5f;
                sprite.setScale(sf::Vector2f(
                    base_scale_x_ * (1.f + 0.18f * s),
                    base_scale_y_ * (1.f - 0.22f * s)));
            }
            else
            {
                current_anim_state = AnimState::Idle;
                float s = (t - 0.5f) / 0.5f;
                float bounce = (1.f - s) * 0.06f;
                sprite.setScale(sf::Vector2f(
                    base_scale_x_ * (1.f + bounce),
                    base_scale_y_ * (1.f - bounce)));
            }
        }

        updateSpriteRect();

        if (progress >= 1.0f)
        {
            is_animating = false;
            current_anim_state = AnimState::Idle;
            // Commit the logical position after the jump completes
            position = target_position;
            start_lerp_position = position;
            sprite.setOrigin({0.f, 0.f});
            sprite.setRotation(sf::degrees(0.f));
            sprite.setPosition(position);
            hitbox.position = position + hitbox_offset;
            sprite.setScale(sf::Vector2f(base_scale_x_, base_scale_y_));
            updateSpriteRect();
        }
    }
}

bool People::isColliding(const sf::FloatRect &other_hitbox) const
{
    if (is_dead || !is_active)
        return false;
    return hitbox.findIntersection(other_hitbox).has_value();
}

bool People::tryToMove(float dx, float dy, float camera_top_y, const Map &map)
{
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
        future_top < camera_top_y || future_bottom > cam_bottom)
    {
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

    // Sync jump arc positions to match the new lerp positions
    jump_start_pos = position;
    jump_end_pos = target_position;

    Audio().playSFX(AudioSystem::SFX::Jump);

    is_animating = true;
    anim_timer = 0.f;
    current_anim_state = AnimState::Prep;
    updateSpriteRect();

    return true;
}

void People::moveBy(sf::Vector2f offset)
{
    DynamicObject::moveBy(offset);
    target_position += offset;
    start_lerp_position += offset;
    jump_start_pos += offset;
    jump_end_pos += offset;
}

void People::die() { is_dead = true; }

bool People::isDead() const { return is_dead; }


sf::Vector2f People::getStartPosition(int init_cell, int init_lane) const
{
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

void People::resetPosition()
{
    position = getStartPosition();
    sprite.setPosition(position);
    hitbox.position = position + hitbox_offset;

    jump_start_pos = position;
    jump_end_pos = position;

    // Sync logic positions
    target_position = position;
    start_lerp_position = position;

    is_dead = false;
    is_active = true;

    current_anim_state = AnimState::Idle;
    current_dir = Direction::Up;
    is_animating = false;
    sprite.setOrigin({0.f, 0.f});
    sprite.setRotation(sf::degrees(0.f));
    sprite.setScale(sf::Vector2f(base_scale_x_, base_scale_y_));
    updateSpriteRect();
}

void People::snapTargetX(float new_x)
{
    target_position.x = new_x;
    jump_end_pos.x = new_x;
}

sf::FloatRect People::getBounds() const { return sprite.getGlobalBounds(); }

bool People::handleEvent(const sf::Event &event, float camera_top_y,
                         const Map &map)
{
    if (is_dead || !is_active)
        return false;

    const auto *key = event.getIf<sf::Event::KeyPressed>();
    if (!key)
        return false;

    Direction new_dir = current_dir;
    float dx = 0.f, dy = 0.f;
    if (key->code == sf::Keyboard::Key::W || key->code == sf::Keyboard::Key::Up)
    {
        new_dir = Direction::Up;
        dy = -step_size;
    }
    else if (key->code == sf::Keyboard::Key::S ||
             key->code == sf::Keyboard::Key::Down)
    {
        new_dir = Direction::Down;
        dy = step_size;
    }
    else if (key->code == sf::Keyboard::Key::A ||
             key->code == sf::Keyboard::Key::Left)
    {
        new_dir = Direction::Left;
        dx = -step_size;
    }
    else if (key->code == sf::Keyboard::Key::D ||
             key->code == sf::Keyboard::Key::Right)
    {
        new_dir = Direction::Right;
        dx = step_size;
    }
    else
    {
        return false;
    }

    Direction prev_dir = current_dir;
    if (new_dir != current_dir)
        current_dir = new_dir;

    // Chỉ giữ hướng mới nếu cú nhảy thực sự bắt đầu. Nếu phím bị chặn
    // (đang trong animation / chạm biên / va chạm) thì hoàn trả hướng cũ,
    // tránh hiện tượng nhân vật quay hướng khi nhảy không thành công.
    if (tryToMove(dx, dy, camera_top_y, map))
        return true;

    current_dir = prev_dir;
    return false;
}

void People::serialize(std::ostream &os) const
{
    Serialization::write(os, is_dead);
    Serialization::write(os, position.x);
    Serialization::write(os, position.y);
    Serialization::write(os, step_size);
    Serialization::write(os, is_active);

    int dir_int = static_cast<int>(current_dir);
    Serialization::write(os, dir_int);
}

void People::deserialize(std::istream &is)
{
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
    sprite.setOrigin({0.f, 0.f});
    sprite.setRotation(sf::degrees(0.f));

    updateSpriteRect();
    sprite.setPosition(position);
    hitbox.position = position + hitbox_offset;
    // Đồng bộ target & start sau khi load để không có Lerp ghost
    target_position = position;
    start_lerp_position = position;
}