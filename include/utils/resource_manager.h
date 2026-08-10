#pragma once

#include "utils/constants.h"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

class ResourceManager
{
public:
	ResourceManager(const ResourceManager &) = delete;
	ResourceManager &operator=(const ResourceManager &) = delete;

	static ResourceManager &getInstance(); // singleton accessor

	const sf::Texture &getTexture(const std::string &filepath);         // load-or-return the cached texture for a file path
	const sf::Font &getFont(const std::string &filepath);               // load-or-return the cached font for a file path
	const sf::SoundBuffer &getSoundBuffer(const std::string &filepath); // load-or-return the cached sound buffer for a file path

	const sf::Texture &getDynamicObjectTexture(const Constants::DynamicObjectType &type, const Constants::ObjectColor &color = Constants::ObjectColor::None, int cur_frame = 1); // cached procedural texture for a DynamicObject type+color
	const sf::Texture &getStaticObjectTexture(const Constants::StaticObjectType &type, const Constants::ObjectColor &color = Constants::ObjectColor::None);   // cached procedural texture for a StaticObject type+color
	const sf::Texture &getLaneTexture(const Constants::LaneType &type, int texture_variant);                                         // cached procedural texture for a lane type+direction
	const sf::Texture &getTrafficTexture(const Constants::TrafficLightColor &color);                                           // cached texture for the traffic light spritesheet
	const sf::IntRect getTrafficTextureRect(const Constants::TrafficLightColor &color);                                        // texture rect for the given color frame in the spritesheet

	bool hasSpritesheet(const Constants::DynamicObjectType &type) const;
	const sf::Texture &getDynamicObjectSpritesheet(const Constants::DynamicObjectType &type, const Constants::ObjectColor &color);
	sf::IntRect getDynamicObjectSpritesheetRect(const Constants::DynamicObjectType &type, int frame, bool is_stopped = false);

	void clearAll(); // drops file-loaded caches (textures/fonts/sounds); every live sf::Sprite/sf::Text referencing them becomes dangling, so only call this on program exit

private:
	std::unordered_map<std::string, std::unique_ptr<sf::Texture>> textures_;
	std::unordered_map<std::string, std::unique_ptr<sf::Font>> fonts_;
	std::unordered_map<std::string, std::unique_ptr<sf::SoundBuffer>> sound_buffers_;

	// std::map<std::pair<int, int>, sf::Texture> dynamic_texture_cache_;
	// std::map<std::pair<int, int>, sf::Texture> static_texture_cache_;
	// std::map<std::pair<int, int>, sf::Texture> lane_texture_cache_;
	// std::map<int, sf::Texture> traffic_texture_cache_;

	ResourceManager() = default;
};

inline ResourceManager &Resource()
{
	return ResourceManager::getInstance();
}
