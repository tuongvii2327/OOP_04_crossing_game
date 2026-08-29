#include "utils/resource_manager.h"
#include "utils/game_exceptions.h"
#include <algorithm>
#include <iostream>

ResourceManager &ResourceManager::getInstance()
{
	static ResourceManager instance;
	return instance;
}

const sf::Texture &ResourceManager::getTexture(const std::string &filepath)
{
	auto it = textures_.find(filepath);
	if (it != textures_.end())
		return *(it->second);

	auto texture = std::make_unique<sf::Texture>();
	if (!texture->loadFromFile(filepath))
		throw TextureLoadException(filepath);

	return *(textures_.emplace(filepath, std::move(texture)).first->second);
}

const sf::Font &ResourceManager::getFont(const std::string &filepath)
{
	auto it = fonts_.find(filepath);
	if (it != fonts_.end())
		return *(it->second);

	auto font = std::make_unique<sf::Font>();
	if (!font->openFromFile(filepath))
		throw FontLoadException(filepath);

	return *(fonts_.emplace(filepath, std::move(font)).first->second);
}

const sf::SoundBuffer &ResourceManager::getSoundBuffer(const std::string &filepath)
{
	auto it = sound_buffers_.find(filepath);
	if (it != sound_buffers_.end())
		return *(it->second);

	auto buffer = std::make_unique<sf::SoundBuffer>();
	if (!buffer->loadFromFile(filepath))
		throw AudioLoadException(filepath);

	return *(sound_buffers_.emplace(filepath, std::move(buffer)).first->second);
}

sf::Text ResourceManager::createText(const std::string &str, const TextStyle &style, bool is_button_text)
{
	const sf::Font &font = getFont("assets/fonts/VT323-Regular.ttf");
	sf::Text text(font);
	text.setString(str);
	text.setCharacterSize(style.size);
	if (is_button_text)
	{
		text.setFillColor(sf::Color::White);
		text.setOutlineColor(OUTLINE_BUTTON_COLOR);
	}
	else
	{
		text.setFillColor(style.fill);
		text.setOutlineColor(style.outline_color);
	}
	text.setOutlineThickness(style.outline_thickness);
	sf::FloatRect bounds = text.getLocalBounds();
	text.setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
	return text;
}

void ResourceManager::setTextButtonHovered(sf::Text &text, bool hovered)
{
	if (hovered)
	{
		text.setFillColor(FILL_BUTTON_COLOR);
		text.setScale({BUTTON_HOVER_SCALE, BUTTON_HOVER_SCALE});
	}
	else
	{
		text.setFillColor(sf::Color::White);
		text.setScale({1.f, 1.f});
	}
}

void ResourceManager::drawTextWithShadow(sf::RenderWindow& window, const sf::Text& text, float offset)
{
	sf::Text shadow = text;
	shadow.setFillColor(TEXT_SHADOW_COLOR);
	shadow.setOutlineColor(sf::Color::Transparent);
	shadow.move({offset, offset});
	window.draw(shadow);
	window.draw(text);
}

const sf::Texture &
ResourceManager::getDynamicObjectTexture(const Constants::DynamicObjectType &type,
	const Constants::ObjectColor &color)
{
	std::string type_name, color_name;
	switch (type)
	{
	case Constants::DynamicObjectType::Car:
		type_name = "car";
		break;
	case Constants::DynamicObjectType::Truck:
		type_name = "truck";
		break;
	case Constants::DynamicObjectType::Bird:
		type_name = "bird";
		break;
	case Constants::DynamicObjectType::Rabbit:
		type_name = "rabbit";
		break;
	case Constants::DynamicObjectType::People:
		type_name = "people";
		break;
	case Constants::DynamicObjectType::Wood:
		type_name = "wood";
		break;
	default:
		type_name = "car";
		break;
	}
	switch (color)
	{
	case Constants::ObjectColor::Red:
		color_name = "red";
		break;
	case Constants::ObjectColor::Blue:
		color_name = "blue";
		break;
	case Constants::ObjectColor::Green:
		color_name = "green";
		break;
	case Constants::ObjectColor::Grey:
		color_name = "grey";
		break;
	case Constants::ObjectColor::None:
		color_name = "none";
		break;
	default:
		color_name = "none";
		break;
	}

	std::string final_path =
		"assets/textures/" + type_name + "/" + color_name + ".png";
	return getInstance().getTexture(final_path);
}

const sf::Texture &ResourceManager::getStaticObjectTexture(const Constants::StaticObjectType &type,
	const Constants::ObjectColor &color)
{
	std::string type_name, color_name;
	switch (type)
	{
	case Constants::StaticObjectType::Rock:
		type_name = "rock";
		break;
	case Constants::StaticObjectType::Bush:
		type_name = "bush";
		break;
	case Constants::StaticObjectType::Traffic:
		type_name = "traffic";
		break;
	case Constants::StaticObjectType::Coin:
		type_name = "coin";
		break;
	default:
		type_name = "rock";
		break;
	}

	if (type == Constants::StaticObjectType::Traffic)
	{
		color_name = "traffic";
	}
	else
	{
		switch (color)
		{
		case Constants::ObjectColor::Red:
			color_name = "red";
			break;
		case Constants::ObjectColor::Blue:
			color_name = "blue";
			break;
		case Constants::ObjectColor::Green:
			color_name = "green";
			break;
		case Constants::ObjectColor::Grey:
			color_name = "grey";
			break;
		case Constants::ObjectColor::None:
			color_name = "none";
			break;
		default:
			color_name = "none";
			break;
		}
	}
	std::string final_path = "assets/textures/" + type_name + "/" + color_name + ".png";
	return getInstance().getTexture(final_path);
}

const sf::Texture &ResourceManager::getLaneTexture(const Constants::LaneType &type,
	int texture_variant)
{
	// Trường hợp đặc biệt: Đối với Sông (River), biến thể texture sẽ đóng vai trò như current_frame để tạo animation
	if (type == Constants::LaneType::River)
	{
		texture_variant = (texture_variant / 3) % Constants::getNumberOfTextureVariant(type) + 1;
	}
	if (texture_variant < 1 || texture_variant > Constants::getNumberOfTextureVariant(type))
	{
		texture_variant = 1;
	}

	std::string type_name, texture_var;
	texture_var = std::to_string(texture_variant);
	switch (type)
	{
	case Constants::LaneType::Grass:
		type_name = "grass";
		break;
	case Constants::LaneType::Road:
		type_name = "road";
		break;
	case Constants::LaneType::Safe:
		type_name = "safe";
		break;
	case Constants::LaneType::River:
		type_name = "river";
		break;
	default:
		type_name = "grass";
		break;
	}
	std::string final_path = "assets/textures/lane/" + type_name + "_" + texture_var + ".png";
	return getInstance().getTexture(final_path);
}

const sf::Texture &ResourceManager::getTrafficTexture(const Constants::TrafficLightColor &color)
{
	std::string final_path = "assets/textures/traffic/traffic.png";
	return getInstance().getTexture(final_path);
}

const sf::IntRect ResourceManager::getTrafficTextureRect(const Constants::TrafficLightColor &color)
{
	const int FRAME_WIDTH = Constants::SPRITESHEET_FRAME_SIZE;
	const int FRAME_HEIGHT = Constants::SPRITESHEET_FRAME_SIZE;
	int column;
	switch (color)
	{
	case Constants::TrafficLightColor::Red:
		column = 0;
		break;
	case Constants::TrafficLightColor::Yellow:
		column = 1;
		break;
	case Constants::TrafficLightColor::Green:
		column = 2;
		break;
	default:
		column = 2;
		break;
	}
	return sf::IntRect(sf::Vector2i(column * FRAME_WIDTH, 0),
		sf::Vector2i(FRAME_WIDTH, FRAME_HEIGHT));
}

bool ResourceManager::hasSpritesheet(const Constants::DynamicObjectType &type) const
{
	return type == Constants::DynamicObjectType::Truck ||
		   type == Constants::DynamicObjectType::Car ||
		   type == Constants::DynamicObjectType::Rabbit ||
		   type == Constants::DynamicObjectType::Bird;
}

const sf::Texture &
ResourceManager::getDynamicObjectSpritesheet(const Constants::DynamicObjectType &type,
	const Constants::ObjectColor &color)
{
	std::string type_name, color_name;
	switch (type)
	{
	case Constants::DynamicObjectType::Truck:
		type_name = "truck";
		break;
	case Constants::DynamicObjectType::Car:
		type_name = "car";
		break;
	case Constants::DynamicObjectType::Rabbit:
		type_name = "rabbit";
		break;
	case Constants::DynamicObjectType::Bird:
		type_name = "bird";
		break;
	default:
		type_name = "truck";
		break;
	}

	switch (color)
	{
	case Constants::ObjectColor::Red:
		color_name = "red";
		break;
	case Constants::ObjectColor::Blue:
		color_name = "blue";
		break;
	case Constants::ObjectColor::Green:
		color_name = "green";
		break;
	case Constants::ObjectColor::Grey:
		color_name = "grey";
		break;
	default:
		color_name = "none";
		break;
	}
	std::string path = "assets/textures/" + type_name + "/" + color_name + ".png";
	return getTexture(path);
}

sf::IntRect ResourceManager::getDynamicObjectSpritesheetRect(const Constants::DynamicObjectType &type, float time_elapsed, bool is_stopped)
{
	const int FRAME_SIZE = Constants::SPRITESHEET_FRAME_SIZE;
	int numFrames = Constants::getNumberOfFrames(type);
	int idx = static_cast<int>(time_elapsed / Constants::ANIMATION_TICK) % numFrames;
	// Bird sheet (1600x1600) is a 2x2 grid of 800x800 cells: r0c0, r0c1 and r1c0
	// are the flying frames; r1c1 (cell 3) is the standing frame shown while stopped.
	if (type == Constants::DynamicObjectType::Bird && is_stopped)
	{
		idx = 3;
	}
	const sf::Texture &tex = getDynamicObjectSpritesheet(type, Constants::ObjectColor::None);
	int cols = tex.getSize().x / FRAME_SIZE;
	int row = idx / cols;
	int col = idx % cols;

	Constants::SpriteFrameRect frame_rect = Constants::getSpriteFrameRect(type);

	return sf::IntRect(
		sf::Vector2i(col * FRAME_SIZE + frame_rect.offset_x, row * FRAME_SIZE + frame_rect.offset_y),
		sf::Vector2i(frame_rect.content_width, frame_rect.content_height));
}

void ResourceManager::clearAll()
{
	textures_.clear();
	fonts_.clear();
	sound_buffers_.clear();
}