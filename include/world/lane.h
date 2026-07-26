#pragma once

#include <deque>
#include <memory>

#include "../utils/constants.h"
#include <SFML/Graphics.hpp>

// Forward declarations
class DynamicObject;
class StaticObject;

class Lane
{
protected:
    int texture_variant;     //fix trong suốt vòng đời, 1 loại lane có nhiều texture_variant, số texture_variant mỗi loại lane trong Constant::getNumberOfGraphic() ps: không biết đặt tên gì

    sf::Sprite sprite;
    sf::Vector2f position; // top left point of sprite
    int level = 1;         // level of game in current frame
    int direction;         // 1 = move right, -1 = move left
    bool is_active = 0;
    int num_active_object = 0;
    std::deque<std::unique_ptr<DynamicObject>> list_dynamic;

    void moveBy(sf::Vector2f offset);                                                   // move position by offset, update sprite position
    void adjustSprite();                                                                // scale sprite to fit LANE_WIDTH x LANE_HEIGHT
    int randomRange(int start, int end);                                                // random int in [start, end]
    virtual void seedObjects() = 0;                                                     // populate list_dynamic with initial objects matching the lane type
    int pickTextureVariant(const Constants::LaneType& type);

public:
    // constructor, destructor
    Lane(const sf::Vector2f &init_position = {0, 0},
         const int &init_level = 1,
         const int &init_direction = 1,
         const Constants::LaneType &init_type = Constants::LaneType::Grass);
    virtual ~Lane();

    // get, set
    sf::Sprite getSprite() const;
    void setSprite(const sf::Texture &texture);

    sf::Vector2f getPosition() const;
    void setPosition(const sf::Vector2f &pos);

    int getLevel() const;
    void setLevel(int new_level);

    int getDirection() const;
    void setDirection(int direct);

    bool isActive() const;
    void setActiveState(bool state);

    int getNumberOfActive() const;
    void setNumberOfActive(int new_num);

    int getTextureVariant() const;
    void setTextureVariant(int new_variant);

    float getWidthSprite() const;
    float getHeightSprite() const;

    virtual Constants::LaneType getType() const = 0;

    // Other
    void checkActive();                                  // re-marks the lane active every frame
    virtual void updateState(float delta_time);          // advance all objects, recount actives, then spawn
    virtual void render(sf::RenderWindow &window) const; // draw lane sprite and objects
    virtual void spawnLane(const sf::Vector2f &new_position,
                           const int &new_level,
                           const int &new_direction); // reuse this Lane instance for a newly scrolled-in row
    virtual void spawnObject() = 0;                   // probabilistically spawn an object

    bool checkLethalCollision(const sf::FloatRect &player_hitbox) const;

    virtual void serialize(std::ostream &os) const;
    virtual void deserialize(std::istream &is);
};
