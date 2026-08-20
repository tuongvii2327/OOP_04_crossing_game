#pragma once

#include "dynamic_object.h"
#include "world/map.h"
#include <SFML/Graphics.hpp>
#include <vector>

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

    // Dynamic shadow (drawn below the sprite to sell the 2.5D jump illusion)
    sf::CircleShape shadow_;

    // --- Hệ thống hạt bụi tiếp đất (Dust Particle System) ---
    struct Particle
    {
        sf::CircleShape shape;
        sf::Vector2f    velocity;
        float           lifetime;     // Thời gian sống còn lại
        float           max_lifetime; // Tổng thời gian sống
    };
    std::vector<Particle> dust_particles_;
    bool just_landed_ = false; // Bật lên đúng 1 frame khi hạ cánh
    bool has_landed_this_jump_ = false; // Đảm bảo chỉ bắn bụi 1 lần mỗi cú nhảy

    void spawnDust(sf::Vector2f ground_pos); // Tạo bụi tại vị trí gầm chân

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
    Constants::DeathCause death_cause_ = Constants::DeathCause::None;
    float death_anim_timer_ = 0.f; // Bộ đếm thời gian hoạt ảnh chết
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

    void die(Constants::DeathCause cause);
    bool isDead() const;
    Constants::DeathCause getDeathCause() const { return death_cause_; }
    bool isAnimating() const { return is_animating; }
    bool hasJustLanded() const { return just_landed_; } // Bật 1 frame khi chạm đất
    float getAnimDuration() const { return anim_duration; }
    sf::Vector2f getStartPosition(int init_cell = 6, int init_lane = 10) const;
    void resetPosition();

    // Thêm phương thức để truy cập đích đến và set X đích đến
    sf::Vector2f getTargetPosition() const { return target_position; }
    void snapTargetX(float new_x);

    sf::FloatRect getBounds() const;
    bool handleEvent(const sf::Event& event, float camera_top_y, const Map& map);

    void render(sf::RenderWindow& window) const override;

    // ---- Serialization & Deserialzation functions
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
};