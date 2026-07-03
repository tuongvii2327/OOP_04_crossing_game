#include "../include/entities/people.h"
#include "../include/utils/constants.h"
#include <iostream>

People::People()
	: Object(Constants::ObjectType::People, Constants::ObjectColor::None, nullptr),
	  is_dead(false),
	  step_size(static_cast<float>(Constants::LANE_HEIGHT))
{
	resetPosition();
}

People::People(const sf::Texture &texture)
	: Object(Constants::ObjectType::People, Constants::ObjectColor::None, nullptr),
	  is_dead(false),
	  step_size(static_cast<float>(Constants::LANE_HEIGHT))
{
	sprite.setTexture(texture, true);

	float target_size = 90.0f;
	sf::Vector2f tex_size = static_cast<sf::Vector2f>(texture.getSize());
	sprite.setScale({target_size / tex_size.x, target_size / tex_size.y});

	hitbox.size = sf::Vector2f(target_size * 0.7f, target_size * 0.55f);
	hitbox_offset = sf::Vector2f(target_size * 0.15f, target_size * 0.45f);

	resetPosition();
}

People::People(float x, float y)
	: Object(Constants::ObjectType::People, Constants::ObjectColor::None, nullptr),
	  is_dead(false),
	  step_size(static_cast<float>(Constants::LANE_HEIGHT)) // Sửa 50.0f thành LANE_HEIGHT
{
	position = sf::Vector2f(x, y);
	sprite.setPosition(position);
	setHitbox();
}

Constants::ObjectType People::getType() const
{
	return Constants::ObjectType::People;
}

Constants::ObjectColor People::getColor() const
{
	return Constants::ObjectColor::None;
}

void People::updateState(float delta_time)
{
	if (is_dead || !is_active)
		return;

	// process any animations or state changes for the People object here
	// add later: add animation for people
}

bool People::isColliding(const sf::FloatRect &other_hitbox) const
{
	if (is_dead || !is_active)
		return false;
	return hitbox.findIntersection(other_hitbox).has_value();
}

void People::move(float dx, float dy)
{
	if (is_dead)
		return;

	position.x += dx;
	position.y += dy;
	sprite.setPosition(position);
	hitbox.position = position + hitbox_offset;
}

bool People::tryToMove(float dx, float dy, float camera_top_y)
{
	if (is_dead)
		return false;

	float target_x = position.x + dx;
	float target_y = position.y + dy;

	if (target_x < 0 || (target_x + getWidthSprite()) > Constants::WINDOW_WIDTH ||
		target_y < camera_top_y || target_y > (camera_top_y + Constants::WINDOW_HEIGHT))
	{
		return false;
	}

	move(dx, dy);
	return true;
}

void People::die()
{
	is_dead = true;
}

bool People::isDead() const
{
	return is_dead;
}

void People::resetPosition()
{
	float start_x = (Constants::WINDOW_WIDTH - sprite.getGlobalBounds().size.x) / 2.f;

	float start_y = Constants::WINDOW_HEIGHT - (Constants::LANE_HEIGHT * 1.5f) - (sprite.getGlobalBounds().size.y / 2.f);

	position = sf::Vector2f(start_x, start_y);
	sprite.setPosition(position);

	hitbox.position = position + hitbox_offset;

	is_dead = false;
}

sf::FloatRect People::getBounds() const
{
	return sprite.getGlobalBounds();
}

bool People::handleEvent(const sf::Event &event, float camera_top_y)
{
	if (is_dead || !is_active)
		return false;

	const auto* key = event.getIf<sf::Event::KeyPressed>();
	if (!key)
		return false;

	if (key->code == sf::Keyboard::Key::W || key->code == sf::Keyboard::Key::Up)
	{
		if (tryToMove(0.f, -step_size, camera_top_y))
		{
			return true;
		}
	}
	else if (key->code == sf::Keyboard::Key::S || key->code == sf::Keyboard::Key::Down)
	{
		return tryToMove(0.f, step_size, camera_top_y);
	}
	else if (key->code == sf::Keyboard::Key::A || key->code == sf::Keyboard::Key::Left)
	{
		return tryToMove(-step_size, 0.f, camera_top_y);
	}
	else if (key->code == sf::Keyboard::Key::D || key->code == sf::Keyboard::Key::Right)
	{
		return tryToMove(step_size, 0.f, camera_top_y);
	}

	return false;
}
