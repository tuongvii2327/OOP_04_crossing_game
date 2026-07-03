#include "../include/entities/object.h"
#include "../include/utils/resource_manager.h"
#include "../include/world/lane.h"

#pragma region PRIVATE
// update hitbox_position using position and hitbox_offset
void Object::updateHitbox()
{
	hitbox.position = position + hitbox_offset;
}

// flip sprite depending on direction.
void Object::applyFacing()
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
// include #setHitboxSize(), #setHitboxOffset(), -updateHitbox()
void Object::setHitbox()
{
	setHitboxSize();
	setHitboxOffset();
	updateHitbox();
}

// hitbox.size = sprite size * HITBOX_SIZE_RATIO
void Object::setHitboxSize()
{
	hitbox.size = sprite.getGlobalBounds().size * Constants::HITBOX_SIZE_RATIO;
}

// hitbox_offset = (sprite size - hitbox.size) / 2
void Object::setHitboxOffset()
{
	hitbox_offset = (sprite.getGlobalBounds().size - hitbox.size) / 2.f;
}

// move position by offset
// include +setPosition()
void Object::moveBy(sf::Vector2f offset)
{
	setPosition(position + offset);
}

// pick a random speed from SpeedRange depending on type
// speed = random number in [min_speed, max_speed] * (1 + (level - 1) * SPEED_LEVEL_FACTOR)
// include +Lane::getLevel(), Constants::getSpeedRange()
float Object::randomSpeed(const Constants::ObjectType &type) const
{
	int level = current_lane->getLevel();
	static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
	Constants::SpeedRange range = Constants::getSpeedRange(type);
	std::uniform_int_distribution<int> dist(range.min_speed, range.max_speed);
	float base = static_cast<float>(dist(rng));
	float factor = 1.f + static_cast<float>(level - 1) * Constants::SPEED_LEVEL_FACTOR;
	return base * factor;
}

// direction taken from current_lane (1 if no lane attached)
// include +Lane::getDirection()
int Object::calcDirection() const
{
	return (current_lane != nullptr) ? current_lane->getDirection() : 1;
}

// calculate object position depending on current_lane's position
// include +getHeightSprite(), +getWidthSprite()
// include +Lane::getPosition()
sf::Vector2f Object::calcSpawnPosition() const
{
	if (current_lane == nullptr)
		return sf::Vector2f(0.f, 0.f);
	sf::Vector2f lane_position = current_lane->getPosition();
	float spawn_y = lane_position.y + (Constants::LANE_HEIGHT - getHeightSprite()) / 2.f;
	float spawn_x = (direction == 1) ? lane_position.x - getWidthSprite()
	                                 : lane_position.x + Constants::LANE_WIDTH;
	return sf::Vector2f(spawn_x, spawn_y);
}

#pragma region GET TEXTURE
// reference into ResourceManager's cache (stays alive for the sprite's lifetime)
// include +ResourceManager::getObjectTexture()
const sf::Texture &Object::getTexture(const Constants::ObjectType &type, const Constants::ObjectColor &color) const
{
	return ResourceManager::getObjectTexture(type, color);
}
#pragma endregion

// scale size of sprite if its size is larger than CELL_SIZE
// include +getHeightSprite(), +getWidthSprite()
void Object::adjustSprite()
{
	if (getWidthSprite() == 0.f || getHeightSprite() == 0.f)
		return;
	if (getWidthSprite() > Constants::LANE_WIDTH || getHeightSprite() > Constants::LANE_HEIGHT)
	{
		float scaleX = (Constants::LANE_WIDTH - Constants::VISUAL_DISTANCE) / getWidthSprite();
		float scaleY = (Constants::LANE_HEIGHT - Constants::VISUAL_DISTANCE) / getHeightSprite();
		float finalScale = std::min(scaleX, scaleY);
		sprite.setScale({finalScale, finalScale});
	}
}
#pragma endregion

#pragma region PUBLIC

#pragma region CONSTRUCTOR, DESTRUCTOR
// position, hitbox, is_active, direction are calculated & set automactically, sprite will be adjusted & apply facing, subclass must setVelocity
// include #adjustSprite(), #calcDirection(), +calcSpawnPosition(), +setLane(), +setDirection(), +setPosition(), #setHitbox(), +setVelocity(), +setActiveState()
// include +ResourceManager::getObjectTexture()
Object::Object(Constants::ObjectType type, Constants::ObjectColor color, Lane* init_lane)
	: sprite(ResourceManager::getObjectTexture(type, color))
{
	adjustSprite();
	setLane(init_lane);
	setDirection(calcDirection()); // direction from lane, then flips the sprite when moving left
	setPosition(calcSpawnPosition());
	setHitbox();
	setVelocity(sf::Vector2f(0.f, 0.f)); // subclasses set the real velocity once they know their speed
	setActiveState(true);
}

// virtual ~Object() = default;
#pragma endregion

#pragma region GET, SET
sf::Vector2f Object::getPosition() const
{
	return position;
}

// update position & sprite position
// include -updateHitbox(), +getHeightSprite()
// include +Lane::getPosition()
void Object::setPosition(const sf::Vector2f &pos)
{
	position = pos;
	if (current_lane != nullptr)
	{
		position.y = current_lane->getPosition().y + (Constants::LANE_HEIGHT - getHeightSprite()) / 2.f;
	}
	sprite.setPosition(position);
	updateHitbox();
}

sf::Vector2f Object::getVelocity() const
{
	return velocity;
}

void Object::setVelocity(const sf::Vector2f &velo)
{
	velocity = velo;
}

sf::Sprite Object::getSprite() const
{
	return sprite;
}

// include -applyFacing(), #adjustSprite()
void Object::setSprite(const sf::Texture &texture)
{
	sprite.setTexture(texture);
	adjustSprite();
	applyFacing();
}

sf::FloatRect Object::getHitBox() const
{
	return hitbox;
}
// set hitbox in protected

sf::Vector2f Object::getHitBoxOffset() const
{
	return hitbox_offset;
}
// set hitbox_offset in protected

bool Object::isActive() const
{
	return is_active;
}

void Object::setActiveState(const bool &state)
{
	is_active = state;
}

Lane* Object::getLane() const
{
	return current_lane;
}

void Object::setLane(Lane* lane)
{
	current_lane = lane;
}

// 1 = right, -1 = left
int Object::getDirection() const
{
	return direction;
}

// accepts only 1 or -1
// include -applyFacing()
void Object::setDirection(const int &direct)
{
	if (direct == 1 || direct == -1)
		direction = direct;
	applyFacing();
}

float Object::getWidthSprite() const
{
	return sprite.getGlobalBounds().size.x;
}

float Object::getHeightSprite() const
{
	return sprite.getGlobalBounds().size.y;
}
#pragma endregion

#pragma region OTHER
// include +getWidthSprite(), +isActive(), +setActiveState()
bool Object::isOutOfLane()
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
// include +getWidthSprite()
float Object::gapToObstacle(const float &obstacle_position_x, const float &obstacle_width) const
{
	float curr_front_x = (direction == 1) ? (position.x + getWidthSprite()) : (position.x);
	float near_edge = (direction == 1) ? (obstacle_position_x)
	                                   : (obstacle_position_x + obstacle_width);
	return direction * (near_edge - curr_front_x) - Constants::VISUAL_DISTANCE;
}

// draw sprite to screen when active
void Object::render(sf::RenderWindow &window) const
{
	if (is_active)
		window.draw(sprite);
}

// virtual Constants::ObjectType getType() const = 0;
// virtual void updateState(float delta_time) = 0;
// virtual bool isColliding(const sf::FloatRect &other_hitbox) const = 0;
#pragma endregion
#pragma endregion
