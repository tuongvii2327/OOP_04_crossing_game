#pragma once

#include "dynamic_object.h"
#include "world/map.h"
#include <SFML/Graphics.hpp>

class People : public DynamicObject
{
public:
    enum class Direction
    {
        Down = 0,
        Right = 1,
        Up = 2,
        Left = 3
    };
    enum class AnimState
    {
        Idle = 0,
        Prep = 1,
        Jump = 2,
        Land = 3
    };

private:
    static constexpr int FRAME_W = Constants::SPRITESHEET_FRAME_SIZE; // 800
    static constexpr int FRAME_H = Constants::SPRITESHEET_FRAME_SIZE; // 800

    // Jump interpolation
    sf::Vector2f jump_start_pos;
    sf::Vector2f jump_end_pos;
    static constexpr float JUMP_ARC_HEIGHT = 65.f;

    // Lerp / logic position tracking
    sf::Vector2f target_position;
    sf::Vector2f start_lerp_position;

    // Base scale for squash/stretch
    float base_scale_x_;
    float base_scale_y_;

    bool is_dead;
    float step_size;
    Constants::ObjectColor people_color;

    Direction current_dir;
    AnimState current_anim_state;
    float anim_timer;
    float anim_duration;
    bool is_animating;

    void updateSpriteRect();

public:
    People(Constants::ObjectColor color = Constants::ObjectColor::None);
    People(float x, float y, Constants::ObjectColor color = Constants::ObjectColor::None);

    Constants::DynamicObjectType getType() const override;
    Constants::ObjectColor getColor() const override;
    void updateState(float delta_time) override;
    bool isColliding(const sf::FloatRect& other_hitbox) const override;

    bool tryToMove(float dx, float dy, float camera_top_y, const Map& map);
    void moveBy(sf::Vector2f offset);

    void die();
    bool isDead() const;
    bool isAnimating() const { return is_animating; }
    float getAnimDuration() const { return anim_duration; }
    sf::Vector2f getStartPosition(int init_cell = 6, int init_lane = 10) const;
    void resetPosition();

    // Thêm phương thức để truy cập đích đến và set X đích đến
    sf::Vector2f getTargetPosition() const { return target_position; }
    void snapTargetX(float new_x);

    sf::FloatRect getBounds() const;
    bool handleEvent(const sf::Event& event, float camera_top_y, const Map& map);

    // ---- Serialization & Deserialzation functions
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
};