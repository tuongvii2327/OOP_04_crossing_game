#pragma once

#include <iostream>
#include <random>

#include "../utils/constants.h"
#include <SFML/Graphics.hpp>
// #include <SFML/Audio.hpp>

class Lane;

class StaticObject
{
private:
	void updateHitbox(); // update hitbox position using position and hitbox_offset
	void applyFacing();  // flip sprite depending on direction

protected:
	sf::Vector2f position; // top left point of sprite
	sf::Sprite sprite;
	sf::FloatRect hitbox;       // collision rectangle (top left point, width x height)
	sf::Vector2f hitbox_offset; // offset of hitbox top-left from sprite top-left
	Lane* current_lane;
	int direction = 1; // 1 = move right, -1 = move left
	Constants::ObjectColor object_color;

	void setHitbox();       // build hitbox: compute size, then offset, then position
	void setHitboxSize();   // hitbox.size = sprite size * HITBOX_SIZE_RATIO
	void setHitboxOffset(); // hitbox_offset = (sprite size - hitbox.size) / 2

	int calcDirection() const;                                                                                     // direction taken from current_lane
	const sf::Texture& getTexture(const Constants::StaticObjectType& type, const Constants::ObjectColor& color) const; // cached texture for type+color
	void adjustSprite();                                                                                            // scale sprite down to fit inside LANE_WIDTH x LANE_HEIGHT (minus VISUAL_DISTANCE) if it's larger

public:
	// contructor, destructor
	StaticObject(Constants::StaticObjectType init_type, const sf::Vector2f& init_position = sf::Vector2f(0.f, 0.f), Lane* init_lane = nullptr, const Constants::ObjectColor& init_color = Constants::ObjectColor::None); // position, hitbox, direction are calculated & set automatically, sprite will be adjusted & apply facing
	virtual ~StaticObject() = default;

	// get, set
	sf::Vector2f getPosition() const;
	void setPosition(const sf::Vector2f& pos);

	sf::Sprite getSprite() const;
	void setSprite(const sf::Texture& texture);

	sf::FloatRect getHitBox() const;
	// set hitbox in protected

	sf::Vector2f getHitBoxOffset() const;
	// set hitbox_offset in protected

	Lane* getLane() const;
	void setLane(Lane* lane);

	int getDirection() const;             // 1 = right, -1 = left
	void setDirection(const int& direct); // accepts only 1 or -1; re-flips the sprite accordingly

	float getWidthSprite() const;
	float getHeightSprite() const;

	Constants::ObjectColor getColor() const;

	// Other
	virtual void render(sf::RenderWindow& window) const;                                       // draw sprite to screen
	virtual Constants::StaticObjectType getType() const = 0;                                    // object type as a StaticObjectType enum
	virtual void updateState(float delta_time) = 0;                                             // update state for this frame (delta_time: seconds since the previous frame)
	virtual bool isColliding(const sf::FloatRect& other_hitbox) const;                           // true if other hitbox intersects this object's hitbox
	virtual Constants::ObjectEffect getEffect() const { return Constants::ObjectEffect::None; } // effect on People when colliding (future Rock/Bush/Coin override)
};
