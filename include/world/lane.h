#pragma once

#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>

#include "../utils/constants.h"
#include <SFML/Graphics.hpp>

class DynamicObject;
class Vehicle;
class Animal;
class Traffic;

class Lane
{
private:
	sf::Sprite sprite;
	sf::Vector2f position; // top left point of sprite
	int level; // level of game in current frame
	int direction; // 1 = move right, -1 = move left
	std::vector<DynamicObject *> list_obj;
	std::vector<Traffic *> list_traffic;
	bool is_add_new_obj;

protected:
	void adjustSprite();

public:
	Lane(const sf::Texture &texture,
		const sf::Vector2f &init_position = {0, 0},
		const int &init_level = 1,
		const int &init_direction = 1);
	~Lane();

	sf::Sprite getSprite() const;
	void setSprite(const sf::Sprite &new_sprite);

	sf::Vector2f getPosition() const;
	void setPosition(const sf::Vector2f &pos);

	int getLevel() const;
	void setLevel(const int &new_level);

	int getDirection() const;
	void setDirection(const int &direct);

	float getWidthSprite() const;
	float getHeightSprite() const;

	void addObject(DynamicObject *obj);
	void removeObject(DynamicObject *obj);
	void addTraffic(Traffic *t);

	Traffic *getNearestTrafficAhead(const Vehicle &vehicle);
	Vehicle *getNearestVehicleAhead(const Vehicle &vehicle);
	Animal* getNearestAnimalAhead(const Animal& animal);

	void updateState(float delta_time);
	void render(sf::RenderWindow &window) const;
};
