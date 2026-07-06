#pragma once

#include "utils/constants.h"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>


class ResourceManager
{
public:
	ResourceManager(const ResourceManager &) = delete;
	ResourceManager &operator=(const ResourceManager &) = delete;

	static ResourceManager &getInstance(); // singleton accessor

	const sf::Texture &getTexture(const std::string &filepath);         // load-or-return the cached texture for a file path
	const sf::Font &getFont(const std::string &filepath);               // load-or-return the cached font for a file path
	const sf::SoundBuffer &getSoundBuffer(const std::string &filepath); // load-or-return the cached sound buffer for a file path

	static const sf::Texture &getDynamicObjectTexture(const Constants::DynamicObjectType &type, const Constants::ObjectColor &color); // cached procedural texture for a DynamicObject type+color
	static const sf::Texture &getStaticObjectTexture(const Constants::StaticObjectType &type, const Constants::ObjectColor &color);   // cached procedural texture for a StaticObject type+color
	static const sf::Texture &getLaneTexture(const Constants::LaneType &type, int direction);                                        // cached procedural texture for a lane type+direction
	static const sf::Texture &getTrafficTexture(const Constants::TrafficLightColor &color);                                          // cached procedural texture for a traffic light color

	void clearAll(); // drops file-loaded caches (textures/fonts/sounds); every live sf::Sprite/sf::Text referencing them becomes dangling, so only call this on program exit

private:
	std::unordered_map<std::string, std::unique_ptr<sf::Texture>> textures_;
	std::unordered_map<std::string, std::unique_ptr<sf::Font>> fonts_;
	std::unordered_map<std::string, std::unique_ptr<sf::SoundBuffer>> sound_buffers_;

	ResourceManager() = default;

#pragma region DEMO PROCEDURAL DRAWING
	static sf::Color toSFColor(const Constants::ObjectColor &color);             // ObjectColor -> concrete sf::Color
	static sf::Color toTrafficColor(const Constants::TrafficLightColor &color);  // TrafficLightColor -> concrete sf::Color
	static sf::Color shade(sf::Color base, float factor);    // multiply channels by factor (>1 lightens, <1 darkens), clamped

	static sf::Texture makeRoadTexture(sf::Color asphalt, int direction);
	static sf::Texture makeGrassTexture(sf::Color base);
	static sf::Texture makeVehicleTexture(unsigned w, unsigned h, sf::Color body, bool isTruck);
	static sf::Texture makeAnimalTexture(unsigned w, unsigned h, sf::Color body, bool isBird);
	static sf::Texture makePersonTexture(sf::Color body);
	static sf::Texture makeRockTexture(unsigned w, unsigned h, sf::Color body);
	static sf::Texture makeBushTexture(unsigned w, unsigned h, sf::Color body);
	static sf::Texture makeTrafficTexture(Constants::TrafficLightColor active);
	static sf::Texture buildDynamicObjectTexture(const Constants::DynamicObjectType &type, const Constants::ObjectColor &color); // dispatch to the right makeXxxTexture() by DynamicObjectType
	static sf::Texture buildStaticObjectTexture(const Constants::StaticObjectType &type, const Constants::ObjectColor &color);   // dispatch to the right makeXxxTexture() by StaticObjectType
	static sf::Texture buildLaneTexture(const Constants::LaneType &type, int direction);                                        // dispatch to makeRoadTexture()/makeGrassTexture() by LaneType
#pragma endregion
};

inline ResourceManager &Resource() { return ResourceManager::getInstance(); }
