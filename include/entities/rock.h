#pragma once

#include "static_object.h"

class Rock : public StaticObject
{
private:
	Constants::ObjectColor rock_color;
public:
	// contructor, destructor
	Rock(Lane* init_lane = nullptr,
		const sf::Vector2f &init_position = sf::Vector2f(0.f, 0.f),
		const Constants::ObjectColor& init_color = Constants::ObjectColor::None);

	// get, set
	Constants::StaticObjectType getType() const override;
	void setPosition(const sf::Vector2f &pos) override;

	// Other
	Constants::ObjectEffect getEffect() const override { return Constants::ObjectEffect::Blocked; }
	void updateState(float delta_time) override;
};
