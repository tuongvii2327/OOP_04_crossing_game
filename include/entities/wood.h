#pragma once

#include "dynamic_object.h"

class Wood : public DynamicObject
{
private:
	Constants::ObjectColor wood_color;

protected:
	float base_speed;    // max speed of this wood
	float safe_distance; // distance to brake or accelerate

	float calcSafeDistance() const; // safe_distance = base_speed * SAFE_DISTANCE_FACTOR

public:
	// constructor, destructor
	Wood(const Constants::ObjectColor &init_color = Constants::ObjectColor::None, Lane *init_lane = nullptr);

	// get, set
	Constants::ObjectColor getColor() const override;
	void setColor(const Constants::ObjectColor &new_color);

	Constants::DynamicObjectType getType() const override;
	Constants::ObjectEffect getEffect() const override { return Constants::ObjectEffect::None; }

	float getBaseSpeed() const;
	void setBaseSpeed(const float &new_speed);

	float getSafeDistance() const;

	// Other
	void updateState(float delta_time) override;
};
