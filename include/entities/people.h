#pragma once

#include "dynamic_object.h"
#include <SFML/Graphics.hpp>

class People : public DynamicObject
{
private:
	bool is_dead;
	float step_size;

public:
	// Constructors
	People();
	explicit People(const sf::Texture &texture);
	People(float x, float y);

	Constants::DynamicObjectType getType() const override;
	Constants::ObjectColor getColor() const override;
	void updateState(float delta_time) override;
	bool isColliding(const sf::FloatRect &other_hitbox) const override;

	bool tryToMove(float dx, float dy, float camera_top_y);
	void move(float dx, float dy);

	void die();
	bool isDead() const;
	void resetPosition();

	sf::FloatRect getBounds() const;
	bool handleEvent(const sf::Event &event, float camera_top_y);
};
