#include "utils/resource_manager.h"
#include <algorithm>
#include <iostream>

#pragma region PRIVATE
#pragma region DEMO PROCEDURAL DRAWING
// ObjectColor -> concrete sf::Color
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

// TrafficLightColor -> concrete sf::Color
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

// multiply channels by factor (>1 lightens, <1 darkens), clamped to [0,255]
sf::Color ResourceManager::shade(sf::Color base, float factor)
{
	auto scaleChannel = [factor](std::uint8_t channel) -> std::uint8_t {
		int scaled = static_cast<int>(static_cast<float>(channel) * factor);
		return static_cast<std::uint8_t>(std::clamp(scaled, 0, 255));
	};
	return sf::Color(scaleChannel(base.r), scaleChannel(base.g), scaleChannel(base.b), base.a);
}

// `direction` is intentionally unused: the road texture no longer draws a
// direction chevron (it was visually confusable with a traffic lamp), so both
// directions of a Road lane share the same look. Kept in the signature/cache
// key for compatibility with buildLaneTexture()/getLaneTexture().
sf::Texture ResourceManager::makeRoadTexture(sf::Color asphalt, int direction)
{
	(void)direction;
	sf::RenderTexture rt;
	(void)rt.resize({Constants::LANE_WIDTH, Constants::LANE_HEIGHT});
	rt.clear(asphalt);

	float w = static_cast<float>(Constants::LANE_WIDTH);
	float h = static_cast<float>(Constants::LANE_HEIGHT);

	sf::Color edgeColor(210, 205, 190, 150);
	sf::RectangleShape topEdge({w, 3.f});
	topEdge.setPosition({0.f, 5.f});
	topEdge.setFillColor(edgeColor);
	rt.draw(topEdge);

	sf::RectangleShape bottomEdge({w, 3.f});
	bottomEdge.setPosition({0.f, h - 8.f});
	bottomEdge.setFillColor(edgeColor);
	rt.draw(bottomEdge);

	// dashed center line
	sf::Color dashColor(230, 205, 60, 210);
	const float dashWidth = 26.f;
	const float dashGap = 22.f;
	const float dashHeight = 4.f;
	for (float x = 10.f; x < w; x += dashWidth + dashGap)
	{
		sf::RectangleShape dash({dashWidth, dashHeight});
		dash.setPosition({x, h * 0.5f - dashHeight * 0.5f});
		dash.setFillColor(dashColor);
		rt.draw(dash);
	}

	rt.display();
	return rt.getTexture();
}

// grass base fill with a scattered grid of small triangular tufts so it
// reads as a texture rather than a flat color
sf::Texture ResourceManager::makeGrassTexture(sf::Color base)
{
	sf::RenderTexture rt;
	(void)rt.resize({Constants::LANE_WIDTH, Constants::LANE_HEIGHT});
	rt.clear(base);

	float w = static_cast<float>(Constants::LANE_WIDTH);
	float h = static_cast<float>(Constants::LANE_HEIGHT);
	sf::Color tuftDark = shade(base, 0.8f);
	sf::Color tuftLight = shade(base, 1.25f);

	const float spacingX = 34.f;
	const float spacingY = 26.f;
	int row = 0;
	for (float y = 6.f; y < h; y += spacingY)
	{
		float offset = (row % 2 == 0) ? 0.f : spacingX * 0.5f;
		for (float x = offset + 6.f; x < w; x += spacingX)
		{
			sf::CircleShape tuft(4.f, 3); // 3 points -> little triangle "blade"
			tuft.setFillColor((row % 2 == 0) ? tuftDark : tuftLight);
			tuft.setPosition({x, y});
			rt.draw(tuft);
		}
		++row;
	}

	rt.display();
	return rt.getTexture();
}

// simple top-down car/truck silhouette: chassis, cabin/cargo block, a
// windshield, and two wheels
sf::Texture ResourceManager::makeVehicleTexture(unsigned w, unsigned h, sf::Color body, bool isTruck)
{
    (void)w;
    (void)h;
    (void)body;

    if (isTruck)
    {
        return getInstance().getTexture("assets/textures/truck_1.png");
    }
    else
    {
        return getInstance().getTexture("assets/textures/car_1.png");
    }
}
// bird: oval body + wing + beak + eye; rabbit: oval body + two ears + tail + eye
sf::Texture ResourceManager::makeAnimalTexture(unsigned w, unsigned h, sf::Color body, bool isBird)
{
    (void)w;
    (void)h;
    (void)body;

    if (isBird)
    {
        return getInstance().getTexture("assets/textures/bird_1.png");
    }
    else
    {
        return getInstance().getTexture("assets/textures/rabbit_1.png");
    }
}

// stick-figure placeholder: head, torso, arms, legs
sf::Texture ResourceManager::makePersonTexture(sf::Color body)
{
	sf::RenderTexture rt;
	(void)rt.resize({50, 70});
	rt.clear(sf::Color::Transparent);

	sf::Color skin(235, 200, 170);
	sf::Color darker = shade(body, 0.75f);
	sf::Color legColor(60, 60, 65);

	sf::CircleShape head(9.f);
	head.setFillColor(skin);
	head.setPosition({16.f, 2.f});
	rt.draw(head);

	sf::RectangleShape torso({22.f, 28.f});
	torso.setPosition({14.f, 22.f});
	torso.setFillColor(body);
	rt.draw(torso);

	sf::RectangleShape armLeft({6.f, 24.f});
	armLeft.setPosition({6.f, 24.f});
	armLeft.setFillColor(darker);
	rt.draw(armLeft);

	sf::RectangleShape armRight({6.f, 24.f});
	armRight.setPosition({38.f, 24.f});
	armRight.setFillColor(darker);
	rt.draw(armRight);

	sf::RectangleShape legLeft({8.f, 18.f});
	legLeft.setPosition({16.f, 50.f});
	legLeft.setFillColor(legColor);
	rt.draw(legLeft);

	sf::RectangleShape legRight({8.f, 18.f});
	legRight.setPosition({26.f, 50.f});
	legRight.setFillColor(legColor);
	rt.draw(legRight);

	rt.display();
	return rt.getTexture();
}

// irregular polygon silhouette with a lighter facet and a crack line for depth
sf::Texture ResourceManager::makeRockTexture(unsigned w, unsigned h, sf::Color body)
{
	sf::RenderTexture rt;
	(void)rt.resize({w, h});
	rt.clear(sf::Color::Transparent);

	float fw = static_cast<float>(w);
	float fh = static_cast<float>(h);
	sf::Color darker = shade(body, 0.7f);
	sf::Color lighter = shade(body, 1.25f);

	sf::ConvexShape rock(7);
	rock.setPoint(0, {fw * 0.10f, fh * 0.70f});
	rock.setPoint(1, {fw * 0.05f, fh * 0.40f});
	rock.setPoint(2, {fw * 0.28f, fh * 0.10f});
	rock.setPoint(3, {fw * 0.60f, fh * 0.05f});
	rock.setPoint(4, {fw * 0.92f, fh * 0.35f});
	rock.setPoint(5, {fw * 0.88f, fh * 0.75f});
	rock.setPoint(6, {fw * 0.45f, fh * 0.95f});
	rock.setFillColor(body);
	rt.draw(rock);

	sf::ConvexShape highlight(4);
	highlight.setPoint(0, {fw * 0.30f, fh * 0.20f});
	highlight.setPoint(1, {fw * 0.55f, fh * 0.12f});
	highlight.setPoint(2, {fw * 0.50f, fh * 0.35f});
	highlight.setPoint(3, {fw * 0.30f, fh * 0.40f});
	highlight.setFillColor(lighter);
	rt.draw(highlight);

	sf::RectangleShape crack({fw * 0.35f, fh * 0.04f});
	crack.setPosition({fw * 0.35f, fh * 0.55f});
	crack.setRotation(sf::degrees(-15.f));
	crack.setFillColor(darker);
	rt.draw(crack);

	rt.display();
	return rt.getTexture();
}

// cluster of overlapping circular "leaf" clumps
sf::Texture ResourceManager::makeBushTexture(unsigned w, unsigned h, sf::Color body)
{
	sf::RenderTexture rt;
	(void)rt.resize({w, h});
	rt.clear(sf::Color::Transparent);

	float fw = static_cast<float>(w);
	float fh = static_cast<float>(h);
	sf::Color darker = shade(body, 0.75f);
	sf::Color lighter = shade(body, 1.2f);

	struct Lump { float cx, cy, r; };
	const Lump lumps[] = {
		{fw * 0.30f, fh * 0.55f, fh * 0.32f},
		{fw * 0.62f, fh * 0.50f, fh * 0.38f},
		{fw * 0.45f, fh * 0.30f, fh * 0.30f},
		{fw * 0.78f, fh * 0.62f, fh * 0.24f},
		{fw * 0.18f, fh * 0.68f, fh * 0.22f},
	};

	for (const auto &lump : lumps)
	{
		sf::CircleShape leaf(lump.r);
		leaf.setFillColor(darker);
		leaf.setPosition({lump.cx - lump.r, lump.cy - lump.r});
		rt.draw(leaf);
	}
	for (const auto &lump : lumps)
	{
		float r = lump.r * 0.8f;
		sf::CircleShape leaf(r);
		leaf.setFillColor(body);
		leaf.setPosition({lump.cx - r, lump.cy - r - lump.r * 0.15f});
		rt.draw(leaf);
	}

	sf::CircleShape speck(fh * 0.06f);
	speck.setFillColor(lighter);
	speck.setPosition({fw * 0.4f, fh * 0.2f});
	rt.draw(speck);

	rt.display();
	return rt.getTexture();
}

// horizontal post: a thin metal bar with a small square housing at the right
// end holding a single lamp, lit in whatever color is currently active.
// StaticObject::applyFacing() mirrors this for direction == -1, so this base
// (unmirrored, used for direction == 1) puts the housing on the right.

sf::Texture ResourceManager::makeTrafficTexture(Constants::TrafficLightColor active)
{
    switch (active)
    {
    case Constants::TrafficLightColor::Green:
        return getInstance().getTexture("assets/textures/traffic_green.png");
    case Constants::TrafficLightColor::Yellow:
        return getInstance().getTexture("assets/textures/traffic_yellow.png");
    case Constants::TrafficLightColor::Red:
    default:
        return getInstance().getTexture("assets/textures/traffic_red.png");
    }
}

// dispatch to the right makeXxxTexture() by DynamicObjectType
sf::Texture ResourceManager::buildDynamicObjectTexture(const Constants::DynamicObjectType &type, const Constants::ObjectColor &color)
{
	sf::Color sfColor = toSFColor(color);
	switch (type)
	{
	case Constants::DynamicObjectType::Car:
		return makeVehicleTexture(60, 36, sfColor, false);
	case Constants::DynamicObjectType::Truck:
		return makeVehicleTexture(88, 42, sfColor, true);
	case Constants::DynamicObjectType::Bird:
		return makeAnimalTexture(46, 34, sfColor, true);
	case Constants::DynamicObjectType::Rabbit:
		return makeAnimalTexture(44, 40, sfColor, false);
	case Constants::DynamicObjectType::People:
	default:
		// People() (no explicit texture) falls back to this placeholder; every
		// real code path uses People(const sf::Texture&) with player.png instead.
		return makePersonTexture(sf::Color{230, 230, 230});
	}
}

// `color` is intentionally unused below: Rock/Bush each ship a single fixed
// look for now, so any color (including None) resolves to the same texture.
// getStaticObjectTexture() still caches by (type, color), which just means a
// few redundant identical cache entries until a StaticObjectType actually
// needs multiple colors.
sf::Texture ResourceManager::buildStaticObjectTexture(const Constants::StaticObjectType &type, const Constants::ObjectColor &color)
{
	switch (type)
	{
	case Constants::StaticObjectType::Rock:
		return makeRockTexture(50, 40, sf::Color{120, 120, 125});
	case Constants::StaticObjectType::Bush:
		return makeBushTexture(46, 46, sf::Color{60, 140, 60});
	default:
		return makeTrafficTexture(Constants::TrafficLightColor::Green);
	}
}

// dispatch to makeRoadTexture()/makeGrassTexture() by LaneType
sf::Texture ResourceManager::buildLaneTexture(const Constants::LaneType &type, int direction)
{
	if (type == Constants::LaneType::Road)
		return makeRoadTexture(sf::Color{44, 50, 56}, direction);
	return makeGrassTexture({58, 150, 55});
}
#pragma endregion
#pragma endregion

#pragma region PUBLIC
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

const sf::Texture &ResourceManager::getDynamicObjectTexture(const Constants::DynamicObjectType &type, const Constants::ObjectColor &color)
{
	static std::map<std::pair<int, int>, sf::Texture> cache;
	std::pair<int, int> key{static_cast<int>(type), static_cast<int>(color)};
	if (cache.find(key) == cache.end())
		cache.emplace(key, buildDynamicObjectTexture(type, color));
	return cache[key];
}

const sf::Texture &ResourceManager::getStaticObjectTexture(const Constants::StaticObjectType &type, const Constants::ObjectColor &color)
{
	static std::map<std::pair<int, int>, sf::Texture> cache;
	std::pair<int, int> key{static_cast<int>(type), static_cast<int>(color)};
	if (cache.find(key) == cache.end())
		cache.emplace(key, buildStaticObjectTexture(type, color));
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
		cache.emplace(key, makeTrafficTexture(color));
	return cache[key];
}

// only call this when the program is shutting down: every live sf::Sprite/
// sf::Text holding a reference into these caches becomes dangling once cleared
void ResourceManager::clearAll()
{
	textures_.clear();
	fonts_.clear();
	sound_buffers_.clear();
}
#pragma endregion
