#pragma once

#include "static_object.h"

class Coin : public StaticObject
{
private:
	Constants::ObjectColor coin_color;
	bool is_collected_ = false;

public:
	Coin(Lane* init_lane = nullptr,
		 const sf::Vector2f &init_position = sf::Vector2f(0.f, 0.f),
		 const Constants::ObjectColor& init_color = Constants::ObjectColor::None);

	~Coin() override = default;

	// Getters
	Constants::StaticObjectType getType() const override;

	void setPosition(const sf::Vector2f &pos) override;
	bool isCollected() const { return is_collected_; }
	void collect() { is_collected_ = true; }

	// Other
	Constants::ObjectEffect getEffect() const override { return Constants::ObjectEffect::None; }
	void updateState(float delta_time) override;
	void render(sf::RenderWindow &window) const override;
};