#pragma once

#include <iostream>
#include <random>

#include "../utils/constants.h"
#include <SFML/Graphics.hpp>
// #include <SFML/Audio.hpp>

class Lane;

class DynamicObject
{
private:
	void updateHitbox(); // update hitbox_position using position and hitbox_offset
	void applyFacing();  // flip sprite depending on direction.

protected:
	sf::Vector2f position; // top left point of sprite
	sf::Vector2f velocity;
	sf::Sprite sprite;
	sf::FloatRect hitbox;       // collision rectangle (top left point, width x height)
	sf::Vector2f hitbox_offset; // offset of hitbox top-left from sprite top-left
	bool is_active;             // inside map or not
	Lane* current_lane;
	int direction = 1; // 1 = move right, -1 = move left
	int current_frame;

	void setHitbox();       // build hitbox: size, then offset, then position
	void setHitboxSize();   // hitbox.size = sprite size * HITBOX_SIZE_RATIO
	void setHitboxOffset(); // hitbox_offset = (sprite size - hitbox.size) / 2

	void moveBy(sf::Vector2f offset);                                                                                    // move position by offset, update sprite position
	int calcDirection() const;                                                                                           // direction taken from current_lane
	void adjustSprite(const Constants::DynamicObjectType& type);                                                                                                 // scale sprite down to fit it size

public:
	// contructor, destructor
	DynamicObject(const Constants::DynamicObjectType &init_type,
		const Constants::ObjectColor &init_color,
		Lane* init_lane = nullptr); // position, hitbox, is_active, direction are calculated & set automactically, sprite will be adjusted & apply facing
	virtual ~DynamicObject() = default;

	// get, set
	sf::Vector2f getPosition() const;
	void setPosition(const sf::Vector2f &pos);

	sf::Vector2f getVelocity() const;
	void setVelocity(const sf::Vector2f &velo);

	sf::Sprite getSprite() const;
	void setSprite(const sf::Texture &texture, const Constants::DynamicObjectType& type);

	sf::FloatRect getHitBox() const;
	// set hitbox in protected

	sf::Vector2f getHitBoxOffset() const;
	// set hitbox_offset in protected

	bool isActive() const;
	void setActiveState(const bool &state);

	Lane* getLane() const;
	void setLane(Lane* lane);

	int getDirection() const;             // 1 = right, -1 = left
	void setDirection(const int &direct); // accepts only 1 or -1; re-flips the sprite accordingly

	int getCurFrame() const;
	void setCurFrame(int new_frame);

	float getWidthSprite() const;
	float getHeightSprite() const;

	virtual Constants::DynamicObjectType getType() const = 0;                                  // object type as a ObjectType enum
	virtual Constants::ObjectColor getColor() const = 0;                                       // object color as an ObjectColor enum
	virtual Constants::ObjectEffect getEffect() const { return Constants::ObjectEffect::None; } // effect on People when colliding (Vehicle/Animal override to Lethal)

	// Other
	float randomSpeed(const Constants::DynamicObjectType &type) const;                        // pick a random speed from SpeedRange depending on type
	sf::Vector2f calcSpawnPosition() const;                                                    // calculate object position depending on current_lane's position
	bool isOutOfLane();                                                                        // true (and deactivates) once the object scrolls off the playable width
	float gapToObstacle(const float &obstacle_position_x, const float &obstacle_width) const;  // calculated relative distance from this object to obstacle (distance between 2 sprite's edges)
	virtual void render(sf::RenderWindow &window) const;                                       // draw sprite to screen when active
	virtual void updateState(float delta_time) = 0;                                            // update state for this frame (delta_time: seconds since the previous frame)
	virtual bool isColliding(const sf::FloatRect &other_hitbox) const;                          // true if other hitbox intersects this object's hitbox; override only when the effect differs from a plain hitbox test
};
