#include "entities/dynamic_object.h"
#include "utils/resource_manager.h"
#include "world/lane.h"

#pragma region PRIVATE
// update hitbox_position using position and hitbox_offset
void DynamicObject::updateHitbox()
{
	hitbox.position = position + hitbox_offset;
}

// flip sprite depending on direction.
void DynamicObject::applyFacing()
{
	sf::Vector2f currentScale = sprite.getScale();
	float absX = std::abs(currentScale.x);

	float width = sprite.getLocalBounds().size.x;

	if (direction == -1)
	{
		sprite.setScale({-absX, currentScale.y});
		sprite.setOrigin({width, 0.f});
	}
	else
	{
		sprite.setScale({absX, currentScale.y});
		sprite.setOrigin({0.f, 0.f});
	}
}
#pragma endregion

#pragma region PROTECTED
// build hitbox: compute size, then offset, then position
void DynamicObject::setHitbox()
{
	setHitboxSize();
	setHitboxOffset();
	updateHitbox();
}

// hitbox.size = sprite size * HITBOX_SIZE_RATIO
void DynamicObject::setHitboxSize()
{
	hitbox.size = sprite.getGlobalBounds().size * Constants::HITBOX_SIZE_RATIO;
}

// hitbox_offset = (sprite size - hitbox.size) / 2
void DynamicObject::setHitboxOffset()
{
	hitbox_offset = (sprite.getGlobalBounds().size - hitbox.size) / 2.f;
}

// direction taken from current_lane (1 if no lane attached)
int DynamicObject::calcDirection() const
{
	return (current_lane != nullptr) ? current_lane->getDirection() : 1;
}

// scale size of sprite if its size is larger than CELL_SIZE
void DynamicObject::adjustSprite(const Constants::DynamicObjectType &type)
{
	if (sprite.getLocalBounds().size.x == 0.f || sprite.getLocalBounds().size.y == 0.f)
		return;
	float scaleX = Constants::getSizeInGame(type).width / sprite.getLocalBounds().size.x;
	float scaleY = Constants::getSizeInGame(type).height / sprite.getLocalBounds().size.y;
	// float finalScale = std::min(scaleX, scaleY);
	sprite.setScale({scaleX, scaleY});
}
#pragma endregion

#pragma region PUBLIC

#pragma region CONSTRUCTOR, DESTRUCTOR
// position, hitbox, is_active, direction are calculated & set automactically, sprite will be adjusted & apply facing, subclass must setVelocity
DynamicObject::DynamicObject(const Constants::DynamicObjectType &init_type,
	const Constants::ObjectColor &init_color,
	Lane* init_lane)
	: sprite(Resource().hasSpritesheet(init_type)
		? Resource().getDynamicObjectSpritesheet(init_type, init_color)
		: Resource().getDynamicObjectTexture(init_type, init_color, 1))
{
	setCurFrame(1);
	if (Resource().hasSpritesheet(init_type))
	{
		setTextureRect(Resource().getDynamicObjectSpritesheetRect(init_type, 1));
	}
	adjustSprite(init_type);
	setLane(init_lane);
	setDirection(calcDirection());
	setPosition(calcSpawnPosition());
	setHitbox();
	setVelocity(sf::Vector2f(0.f, 0.f));
	setActiveState(true);
}
#pragma endregion

#pragma region GET, SET
sf::Vector2f DynamicObject::getPosition() const
{
	return position;
}

// update position & sprite position
void DynamicObject::setPosition(const sf::Vector2f &pos)
{
	position = pos;
	if (current_lane != nullptr)
	{
		position.y = current_lane->getPosition().y + (Constants::LANE_HEIGHT - getHeightSprite()) / 2.f;
	}
	sprite.setPosition(position);
	updateHitbox();
}

sf::Vector2f DynamicObject::getVelocity() const
{
	return velocity;
}

void DynamicObject::setVelocity(const sf::Vector2f &velo)
{
	velocity = velo;
}

sf::Sprite DynamicObject::getSprite() const
{
	return sprite;
}

// swap texture, re-adjust scale/facing, and rebuild the hitbox
void DynamicObject::setSprite(const sf::Texture &texture, const Constants::DynamicObjectType &type)
{
	sprite.setTexture(texture, 1);
	adjustSprite(type);
	applyFacing();
	setHitbox(); // new texture may differ in size from the old one
}

void DynamicObject::setTextureRect(const sf::IntRect &rect)
{
	sprite.setTextureRect(rect);
}

sf::FloatRect DynamicObject::getHitBox() const
{
	return hitbox;
}
// set hitbox in protected

sf::Vector2f DynamicObject::getHitBoxOffset() const
{
	return hitbox_offset;
}
// set hitbox_offset in protected

bool DynamicObject::isActive() const
{
	return is_active;
}

void DynamicObject::setActiveState(const bool &state)
{
	is_active = state;
}

Lane* DynamicObject::getLane() const
{
	return current_lane;
}

void DynamicObject::setLane(Lane* lane)
{
	current_lane = lane;
}

// 1 = right, -1 = left
int DynamicObject::getDirection() const
{
	return direction;
}

// accepts only 1 or -1
void DynamicObject::setDirection(const int &direct)
{
	if (direct == 1 || direct == -1)
		direction = direct;
	applyFacing();
}

int DynamicObject::getCurFrame() const
{
	return current_frame;
}

void DynamicObject::setCurFrame(int new_frame)
{
	current_frame = new_frame;
}

float DynamicObject::getWidthSprite() const
{
	return sprite.getGlobalBounds().size.x;
}

float DynamicObject::getHeightSprite() const
{
	return sprite.getGlobalBounds().size.y;
}
#pragma endregion

#pragma region OTHER

// move position by offset
void DynamicObject::moveBy(sf::Vector2f offset)
{
	setPosition(position + offset);
}

// pick a random speed from SpeedRange depending on type
// speed = random number in [min_speed, max_speed] * (1 + (level - 1) * SPEED_LEVEL_FACTOR)
float DynamicObject::randomSpeed(const Constants::DynamicObjectType &type) const
{
	int level = current_lane ? current_lane->getLevel() : 1;
	static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
	Constants::SpeedRange range = Constants::getSpeedRange(type);
	std::uniform_int_distribution<int> dist(range.min_speed, range.max_speed);
	float base = static_cast<float>(dist(rng));
	float factor = 1.f + static_cast<float>(level) * Constants::SPEED_LEVEL_FACTOR;
	return base * factor;
}

// calculate object position depending on current_lane's position
sf::Vector2f DynamicObject::calcSpawnPosition() const
{
	if (current_lane == nullptr)
		return sf::Vector2f(0.f, 0.f);
	sf::Vector2f lane_position = current_lane->getPosition();
	float spawn_y = lane_position.y + (Constants::LANE_HEIGHT - getHeightSprite()) / 2.f;
	float spawn_x = (direction == 1) ? lane_position.x - getWidthSprite()
									 : lane_position.x + Constants::LANE_WIDTH;
	return sf::Vector2f(spawn_x, spawn_y);
}

// true (and deactivates) once the object scrolls off the playable width
bool DynamicObject::isOutOfLane()
{
	if (position.x < (Constants::GAME_PLAY_BAR_WIDTH - getWidthSprite()) ||
		position.x > Constants::WINDOW_WIDTH)
	{
		if (isActive())
			setActiveState(false);
		return true;
	}
	return false;
}

// calculated relative distance from this object to obstacle (distance between 2 sprite's edges)
float DynamicObject::gapToObstacle(const float &obstacle_position_x, const float &obstacle_width) const
{
	float curr_front_x = (direction == 1) ? (position.x + getWidthSprite()) : (position.x);
	float near_edge = (direction == 1) ? (obstacle_position_x)
									   : (obstacle_position_x + obstacle_width);
	return direction * (near_edge - curr_front_x) - Constants::VISUAL_DISTANCE;
}

// draw sprite to screen when active
void DynamicObject::render(sf::RenderWindow &window) const
{
	if (is_active)
		window.draw(sprite);
}

// true if other hitbox intersects this object's hitbox
bool DynamicObject::isColliding(const sf::FloatRect &other_hitbox) const
{
	return hitbox.findIntersection(other_hitbox).has_value();
}
#pragma endregion
#pragma endregion
