#pragma once

#include "dynamic_object.h"
#include "world/map.h"
#include <SFML/Graphics.hpp>

class People : public DynamicObject
{
public:
    enum class Direction { Down = 0, Right = 1, Up = 2, Left = 3 };
    enum class AnimState { Idle = 0, Prep = 1, Jump = 2, Land = 3 };

private:
    static constexpr int SPRITESHEET_SIZE = 3200;
    static constexpr int GRID_SIZE = 4;
    static constexpr int FRAME_W = SPRITESHEET_SIZE / GRID_SIZE; // 800
    static constexpr int FRAME_H = SPRITESHEET_SIZE / GRID_SIZE; // 800

    bool is_dead;
    float step_size;
    Constants::ObjectColor people_color;

    Direction current_dir;
    AnimState current_anim_state;
    float anim_timer;
    float anim_duration;
    bool is_animating;

    sf::Vector2f target_position;     // điểm đích logic sau mỗi bước nhảy
    sf::Vector2f start_lerp_position; // vị trí sprite lúc bắt đầu Lerp

    void updateSpriteRect();

public:
    People(Constants::ObjectColor color = Constants::ObjectColor::None);
    People(float x, float y, Constants::ObjectColor color = Constants::ObjectColor::None);

    Constants::DynamicObjectType getType() const override;
    Constants::ObjectColor getColor() const override;
    void updateState(float delta_time) override;
    bool isColliding(const sf::FloatRect &other_hitbox) const override;

    bool tryToMove(float dx, float dy, float camera_top_y, const Map &map);

    void die();
    bool isDead() const;
    bool isAnimating() const;
    sf::Vector2f getStartPosition(int init_cell = 6, int init_lane = 10) const;
    void resetPosition();

    sf::FloatRect getBounds() const;
    bool handleEvent(const sf::Event &event, float camera_top_y, const Map &map);

    void serialize(std::ostream &os) const;
    void deserialize(std::istream &is);
};