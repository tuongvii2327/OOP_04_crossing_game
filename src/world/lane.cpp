#include "../include/world/lane.h"

#include "../include/entities/bird.h"
#include "../include/entities/car.h"
#include "../include/entities/rabbit.h"
#include "../include/entities/traffic_light.h"
#include "../include/entities/truck.h"
#include "../include/entities/rock.h"
#include "../include/entities/bush.h"
#include "../include/utils/resource_manager.h"
#include <limits>

#pragma region PROTECTED
// move position by offset, update sprite position
void Lane::moveBy(sf::Vector2f offset)
{
	setPosition(position + offset);
}

// reference into ResourceManager's cache (stays alive for the sprite's lifetime)
const sf::Texture &Lane::getTexture(const Constants::LaneType &type, int direction) const
{
	return ResourceManager::getLaneTexture(type, direction);
}

// scale sprite up to fill LANE_WIDTH x LANE_HEIGHT if it's smaller
void Lane::adjustSprite()
{
	if (getWidthSprite() == 0.f || getHeightSprite() == 0.f)
		return;
	if (getWidthSprite() > Constants::LANE_WIDTH ||
		getHeightSprite() > Constants::LANE_HEIGHT)
	{
		float scaleX = Constants::LANE_WIDTH / getWidthSprite();
		float scaleY = Constants::LANE_HEIGHT / getHeightSprite();
		sprite.setScale({scaleX, scaleY});
	}
}

// random int in [start, end]
int Lane::randomRange(const int &start, const int &end)
{
	static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
	std::uniform_int_distribution<int> dist(start, end);
	return static_cast<int>(dist(rng));
}

// populate list_dynamic with 2-4 initial objects matching the lane type, spread
// across the lane. Invariant: front() = exit edge, back() = entry edge.
// Sort descending by x*direction so front has the highest x*direction (furthest
// ahead).
void Lane::seedObjects()
{
	int count = std::min(randomRange(1, 2),
		static_cast<int>(Constants::MAX_OBJECT_PER_LANE));

	const float min_gap = Constants::CELL_SIZE * 1.5f; // minimum gap between objects

	for (int i = 0; i < count; ++i)
	{
		Constants::ObjectColor color = static_cast<Constants::ObjectColor>(
			randomRange(0, static_cast<int>(Constants::ObjectColor::None) - 1));
		std::unique_ptr<DynamicObject> obj;
		if (type == Constants::LaneType::Grass)
		{
			obj = (randomRange(0, 1) == 0)
					  ? std::unique_ptr<DynamicObject>(std::make_unique<Bird>(color, this))
					  : std::unique_ptr<DynamicObject>(std::make_unique<Rabbit>(color, this));
		}
		else if (type == Constants::LaneType::Road)
		{
			obj = (randomRange(0, 1) == 0)
					  ? std::unique_ptr<DynamicObject>(std::make_unique<Car>(color, this))
					  : std::unique_ptr<DynamicObject>(std::make_unique<Truck>(color, this));
		}
		else
		{
			continue;
		}

		float obj_width = obj->getWidthSprite();
		float lane_left = position.x, lane_right = position.x + Constants::LANE_WIDTH - obj_width;
		int range = lane_right - lane_left;
		if (range < 0)
		{
			range = 0;
		}
		float x = lane_left;
		bool placed = false;
		for (int tryy = 0; tryy < 10; tryy++)
		{
			x = lane_left + randomRange(0, range);
			bool overlap = false;
			for (auto &other : list_dynamic)
			{
				float other_x = other->getPosition().x;
				float other_width = other->getWidthSprite();
				if (x < other_x + other_width + min_gap && x + other_width + min_gap > other_x)
				{
					overlap = true;
					break;
				}
			}
			if (!overlap)
			{
				placed = true;
				break;
			}
		}

		obj->setPosition({x, 0}); // position.y calculated by DynamicObject
		list_dynamic.push_back(std::move(obj));
	}

	std::sort(list_dynamic.begin(), list_dynamic.end(),
		[this](const std::unique_ptr<DynamicObject> &a, const std::unique_ptr<DynamicObject> &b)
		{
			return a->getPosition().x * direction > b->getPosition().x * direction;
		});
	num_active_object = static_cast<int>(list_dynamic.size());
}

// populate list_traffic with 0-2 lights on Road lanes only.
// Each light is at least 2*CELL_SIZE from either lane edge,
// and lights are at least 3*CELL_SIZE apart from each other.
void Lane::seedTraffic()
{
	if (type != Constants::LaneType::Road)
		return;

	const float margin = 2.f * Constants::CELL_SIZE;
	const float gap_min = 3.f * Constants::CELL_SIZE;
	const float usable = Constants::LANE_WIDTH - 2.f * margin;
	// number of traffic
	int count = randomRange(0, 1);
	if (count == 0)
		return;
	// calculate position
	int slot = usable / (count + 1);

	float x = position.x + margin + slot;
	Constants::TrafficLightColor color =
		(randomRange(0, 1) == 0) ? Constants::TrafficLightColor::Green
								 : Constants::TrafficLightColor::Red;
	traffic = std::make_unique<Traffic>(this, color);
	traffic->setPosition({x, 0}); // position.y calculated by DynamicObject
}
#pragma endregion

#pragma region PUBLIC
// texture, type, direction, level, position, active state are set here; then seed initial objects and traffic
Lane::Lane(const sf::Vector2f &init_position, const int &init_level, const int &init_direction, const Constants::LaneType &init_type)
	: sprite(getTexture(init_type, init_direction))
{
	setType(init_type);
	setDirection(init_direction);
	setLevel(init_level);
	setPosition(init_position);
	setActiveState(1);

	seedObjects();
	seedTraffic();
}

Lane::~Lane() = default;

sf::Sprite Lane::getSprite() const
{
	return sprite;
}

// swap texture, then rescale sprite to fit the lane
void Lane::setSprite(const sf::Texture &texture)
{
	sprite.setTexture(texture);
	adjustSprite();
}

sf::Vector2f Lane::getPosition() const
{
	return position;
}

void Lane::setPosition(const sf::Vector2f &pos)
{
	position = pos;
	position.x = Constants::WINDOW_WIDTH -
				 Constants::LANE_WIDTH; // lane is always right of the sidebar
	sprite.setPosition(position);
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

bool Lane::isActive() const
{
	return is_active;
}

void Lane::setActiveState(const bool &state)
{
	is_active = state;
}

int Lane::getNumberOfActive() const
{
	return num_active_object;
}

void Lane::setNumberOfActive(const int &new_num)
{
	num_active_object =
		std::clamp(new_num, 0, static_cast<int>(Constants::MAX_OBJECT_PER_LANE));
}

float Lane::getWidthSprite() const
{
	return sprite.getGlobalBounds().size.x;
}

float Lane::getHeightSprite() const
{
	return sprite.getGlobalBounds().size.y;
}

Constants::LaneType Lane::getType() const
{
	return type;
}

void Lane::setType(const Constants::LaneType &new_type)
{
	type = new_type;
}
#pragma endregion

#pragma region OTHER
// re-marks the lane active every frame (lanes behind the camera are removed by Map, not deactivated here)
void Lane::checkActive()
{
	// Removing hardcoded screen bounds check.
	// The camera scrolls, so y < 0 is perfectly valid for lanes.
	// Map::update already deletes lanes that fall behind the camera.
	setActiveState(true);
}

// advance traffic light + all objects, recount actives, then spawn
void Lane::updateState(float delta_time)
{
	if (traffic)
		traffic->updateState(delta_time);
	for (auto &o : list_dynamic)
		o->updateState(delta_time);

	// recount active objects (source of truth: objects deactivate themselves in
	// updateState)
	num_active_object = 0;
	for (auto &o : list_dynamic)
		if (o->isActive())
			++num_active_object;

	checkActive();
	spawnObject();
}

// draw lane sprite, then every object, then the traffic light if present
void Lane::render(sf::RenderWindow &window) const
{
	window.draw(sprite);
	for (const auto &o : list_dynamic)
	{
		o->render(window);
	}
	if (traffic)
		traffic->render(window);
}

// reuse this Lane instance for a newly scrolled-in row: reposition, flip
// direction/objects if the direction changed, then reshuffle the traffic
// light to a new random spot
void Lane::spawnLane(const sf::Vector2f &new_position, const int &new_level, const int &new_direction)
{
	setPosition(new_position);
	setLevel(new_level);
	if (new_direction != direction)
	{
		setDirection(new_direction);
		for (auto &o : list_dynamic)
		{
			o->setDirection(new_direction);
			// direction changed => flip travel sign (y stays 0)
			o->setVelocity({-o->getVelocity().x, o->getVelocity().y});
		}
		if (traffic)
			traffic->setDirection(new_direction);
	}

	// shuffle the lane's existing traffic light to a new random spot
	if (traffic)
	{
		float lane_left = position.x;
		int lo = static_cast<int>(0.1f * Constants::LANE_WIDTH);
		int hi = static_cast<int>(0.9f * Constants::LANE_WIDTH);
		traffic->setPosition(
			{lane_left + static_cast<float>(randomRange(lo, hi)), position.y});
	}
}

// probabilistically spawn (or reuse a recycled, same type+color, inactive
// object) once the lane has room and the most-recently-entered object has
// cleared the entry edge
void Lane::spawnObject()
{
	if (num_active_object >= static_cast<int>(Constants::MAX_OBJECT_PER_LANE))
		return;
	if (list_dynamic.empty()) // never dereference back() on an empty deque
		return;
	if (randomRange(0, 1) == 0) // ~25%
		return;

	// Only spawn once the most-recently-entered object (back of the deque)
	// has cleared the entry edge, so a new object won't overlap it.
	DynamicObject* last_obj = list_dynamic.back().get();
	float lane_left = position.x;
	float lane_right = position.x + Constants::LANE_WIDTH;
	float entered_distance = (direction == 1)
								 ? (last_obj->getPosition().x - lane_left)
								 : (lane_right - (last_obj->getPosition().x +
													 last_obj->getWidthSprite()));
	if (entered_distance < static_cast<float>(Constants::CELL_SIZE))
		return;

	Constants::ObjectColor exp_color = static_cast<Constants::ObjectColor>(
		randomRange(0, static_cast<int>(Constants::ObjectColor::None) - 1));
	bool is_spawn = false;

	if (type == Constants::LaneType::Grass)
	{
		Constants::DynamicObjectType exp_type = static_cast<Constants::DynamicObjectType>(
			randomRange(static_cast<int>(Constants::DynamicObjectType::Bird),
				static_cast<int>(Constants::DynamicObjectType::Rabbit)));
		// reuse an inactive object (same type+color) sitting in the exit prefix
		for (auto o = list_dynamic.begin(); o != list_dynamic.end() && !(*o)->isActive();)
		{
			if ((*o)->getType() == exp_type && (*o)->getColor() == exp_color)
			{
				Animal* target = dynamic_cast<Animal*>(o->get());
				if (target)
				{
					target->setActiveState(true);
					target->setPosition(target->calcSpawnPosition());
					target->setSpeed(target->randomSpeed(exp_type));
					target->setVelocity({target->getSpeed() * direction, 0.f});
					target->setMoveState(0);
					target->setMoveState(1);
					std::unique_ptr<DynamicObject> moved = std::move(*o);
					o = list_dynamic.erase(o);
					list_dynamic.push_back(std::move(moved));
					is_spawn = true;
				}
				break;
			}
			++o;
		}
		if (!is_spawn)
		{
			if (exp_type == Constants::DynamicObjectType::Bird)
				list_dynamic.push_back(std::make_unique<Bird>(exp_color, this));
			else
				list_dynamic.push_back(std::make_unique<Rabbit>(exp_color, this));
		}
	}
	else if (type == Constants::LaneType::Road)
	{
		Constants::DynamicObjectType exp_type = static_cast<Constants::DynamicObjectType>(
			randomRange(static_cast<int>(Constants::DynamicObjectType::Car),
				static_cast<int>(Constants::DynamicObjectType::Bird) - 1));
		for (auto o = list_dynamic.begin(); o != list_dynamic.end() && !(*o)->isActive();)
		{
			if ((*o)->getType() == exp_type && (*o)->getColor() == exp_color)
			{
				Vehicle* target = dynamic_cast<Vehicle*>(o->get());
				if (target)
				{
					target->setActiveState(true);
					target->setPosition(target->calcSpawnPosition());
					target->setBaseSpeed(target->randomSpeed(exp_type));
					target->setVelocity({target->getBaseSpeed() * direction, 0.f});
					std::unique_ptr<DynamicObject> moved = std::move(*o);
					o = list_dynamic.erase(o);
					list_dynamic.push_back(std::move(moved));
					is_spawn = true;
				}
				break;
			}
			++o;
		}
		if (!is_spawn)
		{
			if (exp_type == Constants::DynamicObjectType::Car)
				list_dynamic.push_back(std::make_unique<Car>(exp_color, this));
			else
				list_dynamic.push_back(std::make_unique<Truck>(exp_color, this));
		}
	}
}

// a Road lane has at most one traffic light, so there's nothing to search:
// just report it when it's red/yellow, ahead of the vehicle (per lane
// direction), and within the vehicle's safe distance.
Traffic* Lane::getNearestTrafficAhead(const Vehicle &vehicle)
{
	if (!traffic)
		return nullptr;

	Constants::TrafficLightColor color = traffic->getTrafficColor();
	if (color == Constants::TrafficLightColor::Green)
		return nullptr;

	float gap = vehicle.gapToObstacle(traffic->getPosition().x, Constants::TRAFFIC_WIDTH);
	if (gap < 0 || gap > vehicle.getSafeDistance())
		return nullptr;

	return traffic.get();
}

// list_dynamic: sorted descending by x*direction (front = furthest ahead, back =
// most recently entered), and objects never overtake each other, so the
// element immediately before `vehicle` in that order is the only candidate
// for "nearest vehicle ahead". updateState() also runs list_dynamic front-to-back
// each frame, so the ahead vehicle's velocity has already been refreshed for
// this frame by the time we read it here.
Vehicle* Lane::getNearestVehicleAhead(const Vehicle &vehicle)
{
	for (auto it = list_dynamic.begin(); it != list_dynamic.end(); ++it)
	{
		if (it->get() != &vehicle)
			continue;
		if (it == list_dynamic.begin())
			return nullptr; // nothing ahead
		auto ahead_it = std::prev(it);
		if (!(*ahead_it)->isActive())
			return nullptr;
		Vehicle* ahead = dynamic_cast<Vehicle*>(ahead_it->get());
		if (ahead && ahead->getVelocity().x * direction < vehicle.getVelocity().x * direction)
			return ahead;
		return nullptr;
	}
	return nullptr;
}

// closest active animal ahead of `animal` (per lane direction) that is either
// stopped or moving no faster than `animal`
Animal* Lane::getNearestAnimalAhead(const Animal &animal)
{
	Animal* nearest = nullptr;
	float nearest_gap = std::numeric_limits<float>::max();
	for (auto &obj : list_dynamic)
	{
		if (!obj->isActive())
			continue;
		Animal* a = dynamic_cast<Animal*>(obj.get());
		if (!a)
			continue;
		float gap = (a->getPosition().x - animal.getPosition().x) * direction;
		if (gap <= 0.f)
			continue; // behind or same position
		bool is_obstacle = !a->isMove() || a->getVelocity().x * direction <=
											   animal.getVelocity().x * direction;
		if (is_obstacle && gap < nearest_gap)
		{
			nearest_gap = gap;
			nearest = a;
		}
	}
	return nearest;
}

// true if any active object with a Lethal effect is colliding with player_hitbox
bool Lane::checkLethalCollision(const sf::FloatRect &player_hitbox) const
{
	for (const auto &obj : list_dynamic)
	{
		if (!obj->isActive())
			continue;

		if (obj->getEffect() == Constants::ObjectEffect::Lethal && obj->isColliding(player_hitbox))
		{
			return true;
		}
	}
	return false;
}
#pragma endregion
