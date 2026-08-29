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

inline constexpr sf::Color OUTLINE_BUTTON_COLOR(0x00, 0x00, 0x00); // outline color shared by every text — pure black for max contrast
inline constexpr sf::Color FILL_BUTTON_COLOR(0xFF, 0xE6, 0x00);    // fill color for a hovered button text — bright yellow
inline constexpr float BUTTON_HOVER_SCALE = 1.1f;        // fixed grow ratio applied on hover
inline constexpr float TEXT_SHADOW_OFFSET = 4.f;          // pixel offset for drop shadow
inline constexpr sf::Color TEXT_SHADOW_COLOR(0, 0, 0, 180); // semi-transparent black shadow

struct TextStyle
{
	unsigned int size = 40;               // character size in pixels
	sf::Color fill = sf::Color::White;    // fill color used by plain text
	sf::Color outline_color = OUTLINE_BUTTON_COLOR; // outline color
	float outline_thickness = 4.f * size / 40.f;        // outline thickness (thicker for readability)
};

class ResourceManager
{
public:
	ResourceManager(const ResourceManager &) = delete;
	ResourceManager &operator=(const ResourceManager &) = delete;

	static ResourceManager &getInstance(); // singleton accessor

	const sf::Texture &getTexture(const std::string &filepath);         // load-or-return the cached texture for a file path
	const sf::Font &getFont(const std::string &filepath);               // load-or-return the cached font for a file path
	const sf::SoundBuffer &getSoundBuffer(const std::string &filepath); // load-or-return the cached sound buffer for a file path

	sf::Text createText(const std::string &str, const TextStyle &style = {}, bool is_button_text = false); // build a styled text (font loaded internally, origin centered)
	void setTextButtonHovered(sf::Text &text, bool hovered);                                                // toggle a button text between its normal and hovered look
	static void drawTextWithShadow(sf::RenderWindow& window, const sf::Text& text, float offset = TEXT_SHADOW_OFFSET); // draw text with a pixel-art drop shadow

	const sf::Texture &getDynamicObjectTexture(const Constants::DynamicObjectType &type, const Constants::ObjectColor &color = Constants::ObjectColor::None); // cached procedural texture for a DynamicObject type+color
	const sf::Texture &getStaticObjectTexture(const Constants::StaticObjectType &type, const Constants::ObjectColor &color = Constants::ObjectColor::None);                      // cached procedural texture for a StaticObject type+color
	const sf::Texture &getLaneTexture(const Constants::LaneType &type, int texture_variant);                                                                                     // cached procedural texture for a lane type+direction
	const sf::Texture &getTrafficTexture(const Constants::TrafficLightColor &color);                                                                                             // cached texture for the traffic light spritesheet
	const sf::IntRect getTrafficTextureRect(const Constants::TrafficLightColor &color);                                                                                          // texture rect for the given color frame in the spritesheet

	bool hasSpritesheet(const Constants::DynamicObjectType &type) const;
	const sf::Texture &getDynamicObjectSpritesheet(const Constants::DynamicObjectType &type, const Constants::ObjectColor &color);
	sf::IntRect getDynamicObjectSpritesheetRect(const Constants::DynamicObjectType &type, int frame, bool is_stopped = false);

	void clearAll(); // drops file-loaded caches (textures/fonts/sounds); every live sf::Sprite/sf::Text referencing them becomes dangling, so only call this on program exit

private:
	std::unordered_map<std::string, std::unique_ptr<sf::Texture>> textures_;
	std::unordered_map<std::string, std::unique_ptr<sf::Font>> fonts_;
	std::unordered_map<std::string, std::unique_ptr<sf::SoundBuffer>> sound_buffers_;

	ResourceManager() = default;
};

inline ResourceManager &Resource()
{
	return ResourceManager::getInstance();
}
