#pragma once

#include <iostream>
#include <random>
#include <algorithm>
#include <utility>
#include <vector>
#include <deque>

#include "../utils/constants.h"
#include <SFML/Graphics.hpp>

class Object;
class Vehicle;
class Animal;
class Traffic;

class Lane
{
private:
	sf::Sprite sprite;
	sf::Vector2f position; // top left point of sprite
	int level = 1; // level of game in current frame
	int direction; // 1 = move right, -1 = move left
	bool is_active = 0;
	int num_active_object = 0;
	std::deque<Object*> list_obj;
	std::vector<Traffic*> list_traffic;
	Constants::LaneType type;

protected:
	void moveBy(sf::Vector2f offset); // move position by offset, update sprite position
	const sf::Texture& getTexture(const Constants::LaneType& type, int direction) const; // cached lane texture
	void adjustSprite();
	int randomRange(const int& start, const int& end);
	void seedObjects(); // populate list_obj with 2-4 initial objects matching the lane type
	void seedTraffic(); // populate list_traffic with 0-2 lights (Road lanes only)

public:
	Lane(const sf::Vector2f& init_position = { 0, 0 },
		const int& init_level = 1,
		const int& init_direction = 1,
		const Constants::LaneType& init_type = Constants::LaneType::Grass);
	~Lane();

	sf::Sprite getSprite() const;
	void setSprite(const sf::Texture& texture);

	sf::Vector2f getPosition() const;
	void setPosition(const sf::Vector2f& pos);

	int getLevel() const;
	void setLevel(const int& new_level);

	int getDirection() const;
	void setDirection(const int& direct);

	bool isActive() const;
	void setActiveState(const bool& state);

	int getNumberOfActive() const;
	void setNumberOfActive(const int& new_num);

    float getWidthSprite() const;
    float getHeightSprite() const;

	Constants::LaneType getType() const;
	void setType(const Constants::LaneType& new_type);



	void checkActive();
	void updateState(float delta_time);
	void render(sf::RenderWindow& window) const;

	void spawnLane(const sf::Vector2f& new_position, const int& new_level, const int& new_direction);
	void spawnObject();

	Traffic* getNearestTrafficAhead(const Vehicle& vehicle);
	Vehicle* getNearestVehicleAhead(const Vehicle& vehicle);
	Animal* getNearestAnimalAhead(const Animal& animal);
	bool checkLethalCollision(const sf::FloatRect& player_hitbox) const;
};
