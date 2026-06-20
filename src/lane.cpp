#include "../include/world/lane.h"

#include "../include/entities/traffic_light.h"
#include "../include/entities/car.h"
#include "../include/entities/truck.h"
#include "../include/entities/bird.h"
#include "../include/entities/rabit.h"



#pragma region PROTECTED
void Lane::adjustSprite()
{
	if (getWidthSprite() == 0.f || getHeightSprite() == 0.f)
		return;
	if (getWidthSprite() > Constants::LANE_WIDTH || getHeightSprite() > Constants::LANE_HEIGHT)
	{
		float scaleX = Constants::LANE_WIDTH / getWidthSprite();
		float scaleY = Constants::LANE_HEIGHT / getHeightSprite();
		sprite.setScale({scaleX, scaleY});
	}
}

#pragma endregion

#pragma region PUBLIC
Lane::Lane(const sf::Texture &texture,
	const sf::Vector2f &init_position,
	const int &init_level,
	const int &init_direction)
	: sprite(texture)
{
	setPosition(init_position);
	setLevel(init_level);
	setDirection(init_direction);
	is_add_new_obj = 1;
}

Lane::~Lane()
{
	for (auto obj : list_obj)
	{
		delete obj;
		obj = nullptr;
	}
	for (auto obj : list_traffic)
	{
		delete obj;
		obj = nullptr;
	}
}

sf::Sprite Lane::getSprite() const
{
	return sprite;
}

void Lane::setSprite(const sf::Sprite &new_sprite)
{
	sprite = new_sprite;
	adjustSprite();
}

sf::Vector2f Lane::getPosition() const
{
	return position;
}

void Lane::setPosition(const sf::Vector2f &pos)
{
	position = pos;
	sprite.setPosition(pos);
}

int Lane::getLevel() const
{
	return level;
}

void Lane::setLevel(const int &new_level)
{
	level = new_level;
}

int Lane::getDirection() const
{
	return direction;
}

void Lane::setDirection(const int &direct)
{
	if (direct == 1 || direct == -1)
		direction = direct;
}

float Lane::getWidthSprite() const
{
	return sprite.getGlobalBounds().size.x;
}

float Lane::getHeightSprite() const
{
	return sprite.getGlobalBounds().size.y;
}

void Lane::addObject(DynamicObject *obj)
{
	list_obj.push_back(obj);
	is_add_new_obj = 1;
}

void Lane::removeObject(DynamicObject *obj)
{
	auto it = std::find(list_obj.begin(), list_obj.end(), obj);
	if (it != list_obj.end())
	{
		list_obj.erase(it);
	}
}

void Lane::addTraffic(Traffic *t)
{
	list_traffic.push_back(t);
	is_add_new_obj = 1;
}

Traffic *Lane::getNearestTrafficAhead(const Vehicle &vehicle)
{
	float min_distance = (direction == 1)
		? (vehicle.getPosition().x + vehicle.getWidthSprite() + Constants::VISUAL_DISTANCE)
		: (vehicle.getPosition().x - Constants::TRAFFIC_WIDTH - Constants::VISUAL_DISTANCE);
	auto it = std::lower_bound(list_traffic.begin(), list_traffic.end(), min_distance,
		[this](const Traffic *t, float val)
		{
			return t->getPosition().x * direction < val * direction;
		});
	while (it != list_traffic.end())
	{
		if ((*it)->getColor() == Constants::TrafficLightColor::Red)
		{
			return *it;
		}
		if ((*it)->getColor() == Constants::TrafficLightColor::Yellow)
		{
			float position_after_yellow = (Constants::YELLOW_DURATION - (*it)->getTimer()) * vehicle.getVelocity().x + vehicle.getPosition().x;
			float target_position = (direction == 1)
				? ((*it)->getPosition().x + Constants::TRAFFIC_WIDTH + (*it)->getWidthSprite())
				: ((*it)->getPosition().x + Constants::TRAFFIC_WIDTH);
			if (position_after_yellow * direction < target_position * direction)
				return *it;
		}
		++it;
	}
	return nullptr;
}

Vehicle *Lane::getNearestVehicleAhead(const Vehicle &vehicle)
{
	auto it = std::upper_bound(list_obj.begin(), list_obj.end(), vehicle.getPosition().x,
		[this](float val, const DynamicObject *v)
		{
			return val * direction < v->getPosition().x * direction;
		});
	while (it != list_obj.end())
	{
		if ((*it)->isActive() &&
			(*it)->getVelocity().x * direction < vehicle.getVelocity().x * direction)
			return dynamic_cast<Vehicle *>(*it);
		++it;
	}
	return nullptr;
}

Animal* Lane::getNearestAnimalAhead(const Animal& animal)
{
	auto it = std::upper_bound(list_obj.begin(), list_obj.end(), animal.getPosition().x,
		[this](float val, const DynamicObject* a)
		{
			return val * direction < a->getPosition().x * direction;
		});
	while (it != list_obj.end())
	{
		if ((*it)->isActive() &&
			((*it)->getVelocity().x * direction < animal.getVelocity().x * direction || !dynamic_cast<Animal*>(*it)->isMove()))
			return dynamic_cast<Animal*>(*it);
		++it;
	}
	return nullptr;
}

void Lane::updateState(float delta_time)
{
	if (is_add_new_obj)
	{
		is_add_new_obj = 0;
		if (!list_obj.empty())
			std::sort(list_obj.begin(), list_obj.end(),
				[this](const DynamicObject *a, const DynamicObject *b)
				{
					return a->getPosition().x * direction < b->getPosition().x * direction;
				});
		if (!list_traffic.empty())
			std::sort(list_traffic.begin(), list_traffic.end(),
				[this](const Traffic *a, const Traffic *b)
				{
					return a->getPosition().x * direction < b->getPosition().x * direction;
				});
	}
	for (auto v : list_obj)
	{
		v->updateState(delta_time);
	}
	for (auto t : list_traffic)
	{
		t->updateState(delta_time);
	}
}

void Lane::render(sf::RenderWindow &window) const
{
	window.draw(sprite);
	for (auto v : list_obj)
	{
		v->render(window);
	}
	for (auto t : list_traffic)
	{
		t->render(window);
	}
}

#pragma endregion
