#pragma once

#include <iostream>
#include <random>
#include <algorithm>

#include <SFML/Graphics.hpp>

#include "../utils/constants.h"

class Lane;

class Traffic
{
private:
	sf::Sprite sprite;
	sf::Vector2f position;
	Lane *current_lane;
	Constants::TrafficLightColor color;
	float timer;
	float duration;

protected:
	void adjustSprite();
	float randomDuration(Constants::TrafficLightColor color) const;

public:
	Traffic(const sf::Texture &texture,
		Lane *init_lane = nullptr,
		const sf::Vector2f &init_postition = sf::Vector2f{0, 0},
		const Constants::TrafficLightColor &init_color = Constants::TrafficLightColor::Green,
		const float &init_timer = 0.0f);

	sf::Sprite getSprite() const;
	void setSprite(const sf::Sprite &new_sprite);

	sf::Vector2f getPosition() const;
	void setPosition(const sf::Vector2f &pos);

	Lane *getLane() const;
	void setLane(Lane *lane);

	Constants::TrafficLightColor getColor() const;
	void setColor(const Constants::TrafficLightColor &new_color);

	float getTimer() const;
	void setTimer(const float &new_timer);

	float getDuration() const;
	void setDuration(const float &new_duration);

	float getWidthSprite() const;
	float getHeightSprite() const;

	void updateState(float delta_time);
	void render(sf::RenderWindow &window) const;
};
