#include "utils/resource_manager.h"
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
		throw std::runtime_error("[ResourceManager] FATAL: Cannot load Texture -> " + filepath);

	return *(textures_.emplace(filepath, std::move(texture)).first->second);
}

const sf::Font &ResourceManager::getFont(const std::string &filepath)
{
	auto it = fonts_.find(filepath);
	if (it != fonts_.end())
		return *(it->second);

	auto font = std::make_unique<sf::Font>();
	if (!font->openFromFile(filepath))
		throw std::runtime_error("[ResourceManager] FATAL: Cannot load Font -> " + filepath);

	return *(fonts_.emplace(filepath, std::move(font)).first->second);
}

const sf::SoundBuffer &ResourceManager::getSoundBuffer(const std::string &filepath)
{
	auto it = sound_buffers_.find(filepath);
	if (it != sound_buffers_.end())
		return *(it->second);

	auto buffer = std::make_unique<sf::SoundBuffer>();
	if (!buffer->loadFromFile(filepath))
		throw std::runtime_error("[ResourceManager] FATAL: Cannot load SoundBuffer -> " + filepath);

	return *(sound_buffers_.emplace(filepath, std::move(buffer)).first->second);
}

void ResourceManager::clearAll()
{
	textures_.clear();
	fonts_.clear();
	sound_buffers_.clear();
}

sf::Color ResourceManager::toSFColor(const Constants::ObjectColor &color)
{
	switch (color)
	{
	case Constants::ObjectColor::Pink:
		return {210, 55, 55};
	case Constants::ObjectColor::Yellow:
		return {230, 195, 25};
	case Constants::ObjectColor::Orange:
		return {215, 115, 35};
	case Constants::ObjectColor::Blue:
		return {50, 120, 215};
	case Constants::ObjectColor::Green:
		return {55, 175, 70};
	case Constants::ObjectColor::Grey:
		return {160, 160, 165};
	default:
		return {200, 200, 200};
	}
}

sf::Color ResourceManager::toTrafficColor(const Constants::TrafficLightColor &color)
{
	switch (color)
	{
	case Constants::TrafficLightColor::Red:
		return {225, 55, 50};
	case Constants::TrafficLightColor::Yellow:
		return {240, 200, 45};
	default:
		return {60, 205, 85};
	}
}

sf::Texture ResourceManager::makeRoadTexture(sf::Color asphalt, int direction)
{
	sf::RenderTexture rt;
	(void)rt.resize({Constants::LANE_WIDTH, Constants::LANE_HEIGHT});
	rt.clear(asphalt);
	rt.display();
	return rt.getTexture();
}

sf::Texture ResourceManager::makeGrassTexture(sf::Color base)
{
	sf::RenderTexture rt;
	(void)rt.resize({Constants::LANE_WIDTH, Constants::LANE_HEIGHT});
	rt.clear(base);
	rt.display();
	return rt.getTexture();
}

sf::Texture ResourceManager::makeVehicleTexture(unsigned w, unsigned h, sf::Color body, bool isTruck)
{
	sf::RenderTexture rt;
	(void)rt.resize({w, h});
	rt.clear(body);
	rt.display();
	return rt.getTexture();
}

sf::Texture ResourceManager::makeAnimalTexture(unsigned w, unsigned h, sf::Color body, bool isBird)
{
	sf::RenderTexture rt;
	(void)rt.resize({w, h});
	rt.clear(body);
	rt.display();
	return rt.getTexture();
}

sf::Texture ResourceManager::makeTrafficTexture(sf::Color lamp)
{
	sf::RenderTexture rt;
	(void)rt.resize({30, 30});
	rt.clear(lamp);
	rt.display();
	return rt.getTexture();
}

sf::Texture ResourceManager::buildObjectTexture(const Constants::ObjectType &type, const Constants::ObjectColor &color)
{
	sf::Color sfColor = toSFColor(color);
	switch (type)
	{
	case Constants::ObjectType::Car:
		return makeVehicleTexture(60, 36, sfColor, false);
	case Constants::ObjectType::Truck:
		return makeVehicleTexture(88, 42, sfColor, true);
	case Constants::ObjectType::Bird:
		return makeAnimalTexture(46, 34, sfColor, true);
	case Constants::ObjectType::Rabit:
		return makeAnimalTexture(44, 40, sfColor, false);
	default:
		return makeTrafficTexture(toTrafficColor(Constants::TrafficLightColor::Green));
	}
}

sf::Texture ResourceManager::buildLaneTexture(const Constants::LaneType &type, int direction)
{
	if (type == Constants::LaneType::Road)
		return makeRoadTexture(sf::Color{44, 50, 56}, direction);
	return makeGrassTexture({58, 150, 55});
}

const sf::Texture &ResourceManager::getObjectTexture(const Constants::ObjectType &type, const Constants::ObjectColor &color)
{
	static std::map<std::pair<int, int>, sf::Texture> cache;
	std::pair<int, int> key{static_cast<int>(type), static_cast<int>(color)};
	if (cache.find(key) == cache.end())
		cache.emplace(key, buildObjectTexture(type, color));
	return cache[key];
}

const sf::Texture &ResourceManager::getLaneTexture(const Constants::LaneType &type, int direction)
{
	static std::map<std::pair<int, int>, sf::Texture> cache;
	std::pair<int, int> key{static_cast<int>(type), direction};
	if (cache.find(key) == cache.end())
		cache.emplace(key, buildLaneTexture(type, direction));
	return cache[key];
}

const sf::Texture &ResourceManager::getTrafficTexture(const Constants::TrafficLightColor &color)
{
	static std::map<int, sf::Texture> cache;
	int key = static_cast<int>(color);
	if (cache.find(key) == cache.end())
		cache.emplace(key, makeTrafficTexture(toTrafficColor(color)));
	return cache[key];
}
