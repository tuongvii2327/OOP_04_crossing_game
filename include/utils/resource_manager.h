#pragma once

#include <algorithm>
#include <map>
#include <random>
#include <SFML/Graphics.hpp>
#include "constants.h"

// Textures are owned by ResourceManager and handed out by const reference.
// In SFML 3 sf::Sprite only stores a *pointer* to its texture, so the texture
// must outlive every sprite that uses it. Returning references into a static
// cache (instead of fresh sf::Texture values) keeps that pointer valid and
// avoids rebuilding identical textures.
class ResourceManager
{
public:
	// Lane textures
	static sf::Texture makeRoadTexture(sf::Color asphalt, int direction);
	static sf::Texture makeGrassTexture(sf::Color base);

	// Entity textures
	static sf::Texture makeVehicleTexture(unsigned w, unsigned h, sf::Color body, bool isTruck);
	static sf::Texture makeAnimalTexture(unsigned w, unsigned h, sf::Color body, bool isBird);
	static sf::Texture makeTrafficTexture(sf::Color lamp); // lamp = the lit colour

	// Cached lookups (build on first use, then return a stable reference)
	static const sf::Texture& getObjectTexture(const Constants::ObjectType& type, const Constants::ObjectColor& color);
	static const sf::Texture& getLaneTexture(const Constants::LaneType& type, int direction);
	static const sf::Texture& getTrafficTexture(const Constants::TrafficLightColor& color);

private:
	static sf::Color toSFColor(const Constants::ObjectColor& color);
	static sf::Color toTrafficColor(const Constants::TrafficLightColor& color);
	static sf::Texture buildObjectTexture(const Constants::ObjectType& type, const Constants::ObjectColor& color);
	static sf::Texture buildLaneTexture(const Constants::LaneType& type, int direction);
};
