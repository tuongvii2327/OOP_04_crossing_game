#include "../include/utils/resource_manager.h"

// ── Color mapping ──────────────────────────────────────────────────────────

sf::Color ResourceManager::toSFColor(const Constants::ObjectColor& color)
{
	switch (color)
	{
	case Constants::ObjectColor::Pink:   return {210, 55,  55};
	case Constants::ObjectColor::Yellow: return {230, 195, 25};
	case Constants::ObjectColor::Orange: return {215, 115, 35};
	case Constants::ObjectColor::Blue:   return {50,  120, 215};
	case Constants::ObjectColor::Green:  return {55,  175, 70};
	case Constants::ObjectColor::Grey:   return {160, 160, 165};
	default:                             return {200, 200, 200};
	}
}

sf::Color ResourceManager::toTrafficColor(const Constants::TrafficLightColor& color)
{
	switch (color)
	{
	case Constants::TrafficLightColor::Red:    return {225, 55,  50};
	case Constants::TrafficLightColor::Yellow: return {240, 200, 45};
	default:                                   return {60,  205, 85}; // Green
	}
}

// ── Lane textures ──────────────────────────────────────────────────────────

sf::Texture ResourceManager::makeRoadTexture(sf::Color asphalt, int direction)
{
	sf::RenderTexture rt;
	(void)rt.resize({Constants::LANE_WIDTH, Constants::LANE_HEIGHT});
	rt.clear(asphalt);

	// edge highlights
	sf::RectangleShape top({(float)Constants::LANE_WIDTH, 3.f});
	top.setFillColor({255, 255, 255, 18});
	rt.draw(top);
	sf::RectangleShape bot({(float)Constants::LANE_WIDTH, 3.f});
	bot.setFillColor({0, 0, 0, 40});
	bot.setPosition({0.f, (float)Constants::LANE_HEIGHT - 3.f});
	rt.draw(bot);

	// centre dashes
	sf::RectangleShape dash({34.f, 4.f});
	dash.setFillColor({200, 200, 160, 90});
	float y = Constants::LANE_HEIGHT * 0.5f - 2.f;
	for (unsigned x = 10; x + 34 < Constants::LANE_WIDTH; x += 70)
	{
		dash.setPosition({(float)x, y});
		rt.draw(dash);
	}

	// direction arrow painted on the road
	float midY = Constants::LANE_HEIGHT * 0.5f;
	float midX = Constants::LANE_WIDTH  * 0.5f;
	sf::ConvexShape arr(3);
	if (direction == 1)
	{
		arr.setPoint(0, {midX - 18.f, midY - 9.f});
		arr.setPoint(1, {midX + 18.f, midY});
		arr.setPoint(2, {midX - 18.f, midY + 9.f});
	}
	else
	{
		arr.setPoint(0, {midX + 18.f, midY - 9.f});
		arr.setPoint(1, {midX - 18.f, midY});
		arr.setPoint(2, {midX + 18.f, midY + 9.f});
	}
	arr.setFillColor({255, 255, 255, 22});
	rt.draw(arr);

	rt.display();
	return rt.getTexture();
}

sf::Texture ResourceManager::makeGrassTexture(sf::Color base)
{
	sf::RenderTexture rt;
	(void)rt.resize({Constants::LANE_WIDTH, Constants::LANE_HEIGHT});
	rt.clear(base);

	// subtle stripe
	sf::RectangleShape stripe({(float)Constants::LANE_WIDTH, 3.f});
	stripe.setFillColor({0, 0, 0, 30});
	stripe.setPosition({0.f, Constants::LANE_HEIGHT * 0.5f - 1.5f});
	rt.draw(stripe);

	// random blades of grass
	std::mt19937 rng2(reinterpret_cast<size_t>(&base));
	for (int i = 0; i < 90; ++i)
	{
		float gx = (float)(rng2() % Constants::LANE_WIDTH);
		float gy = (float)(rng2() % (Constants::LANE_HEIGHT - 8));
		sf::RectangleShape blade({2.f, 6.f});
		blade.setFillColor({
			(uint8_t)std::clamp((int)base.r - 20, 0, 255),
			(uint8_t)std::clamp((int)base.g - 20, 0, 255),
			(uint8_t)std::clamp((int)base.b - 20, 0, 255),
			130});
		blade.setPosition({gx, gy});
		rt.draw(blade);
	}

	rt.display();
	return rt.getTexture();
}

// ── Entity textures ────────────────────────────────────────────────────────

sf::Texture ResourceManager::makeVehicleTexture(unsigned w, unsigned h,
	sf::Color body, bool isTruck)
{
	sf::RenderTexture rt;
	(void)rt.resize({w, h});
	rt.clear(sf::Color::Transparent);

	auto darker = [](sf::Color c, int d) -> sf::Color
	{
		return {(uint8_t)std::clamp((int)c.r - d, 0, 255),
			(uint8_t)std::clamp((int)c.g - d, 0, 255),
			(uint8_t)std::clamp((int)c.b - d, 0, 255)};
	};

	if (isTruck)
	{
		sf::RectangleShape cargo({(float)w * 0.65f, (float)h});
		cargo.setFillColor(darker(body, 35));
		rt.draw(cargo);
		sf::RectangleShape cab({(float)w * 0.35f, (float)h});
		cab.setFillColor(body);
		cab.setPosition({(float)w * 0.65f, 0.f});
		rt.draw(cab);
		sf::RectangleShape win({(float)w * 0.14f, (float)h * 0.42f});
		win.setFillColor({160, 215, 245, 210});
		win.setPosition({(float)w * 0.68f, (float)h * 0.10f});
		rt.draw(win);
		for (int i = 1; i <= 2; ++i)
		{
			sf::RectangleShape line({1.5f, (float)h * 0.7f});
			line.setFillColor({0, 0, 0, 60});
			line.setPosition({(float)w * 0.65f * i / 3.f, (float)h * 0.15f});
			rt.draw(line);
		}
	}
	else
	{
		sf::RectangleShape b({(float)w, (float)h});
		b.setFillColor(body);
		rt.draw(b);
		sf::RectangleShape roof({(float)w * 0.55f, (float)h * 0.38f});
		roof.setFillColor(darker(body, 20));
		roof.setPosition({(float)w * 0.22f, (float)h * 0.12f});
		rt.draw(roof);
		sf::RectangleShape win({(float)w * 0.22f, (float)h * 0.36f});
		win.setFillColor({160, 215, 245, 210});
		win.setPosition({(float)w * 0.08f, (float)h * 0.13f});
		rt.draw(win);
		win.setPosition({(float)w * 0.70f, (float)h * 0.13f});
		rt.draw(win);
		sf::RectangleShape shadow({(float)w, 5.f});
		shadow.setFillColor({0, 0, 0, 70});
		shadow.setPosition({0.f, (float)h - 5.f});
		rt.draw(shadow);
	}
	rt.display();
	return rt.getTexture();
}

sf::Texture ResourceManager::makeAnimalTexture(unsigned w, unsigned h,
	sf::Color body, bool isBird)
{
	sf::RenderTexture rt;
	(void)rt.resize({w, h});
	rt.clear(sf::Color::Transparent);

	auto darker = [](sf::Color c, int d) -> sf::Color
	{
		return {(uint8_t)std::clamp((int)c.r - d, 0, 255),
			(uint8_t)std::clamp((int)c.g - d, 0, 255),
			(uint8_t)std::clamp((int)c.b - d, 0, 255)};
	};

	if (isBird)
	{
		sf::CircleShape b((float)h * 0.40f);
		b.setFillColor(body);
		b.setPosition({(float)w * 0.20f, (float)h * 0.18f});
		rt.draw(b);
		sf::CircleShape wing((float)h * 0.24f);
		wing.setFillColor(darker(body, 30));
		wing.setPosition({(float)w * 0.32f, (float)h * 0.34f});
		rt.draw(wing);
		sf::ConvexShape beak(3);
		beak.setPoint(0, {(float)w * 0.78f, (float)h * 0.40f});
		beak.setPoint(1, {(float)w * 0.98f, (float)h * 0.50f});
		beak.setPoint(2, {(float)w * 0.78f, (float)h * 0.58f});
		beak.setFillColor({240, 170, 40});
		rt.draw(beak);
		sf::CircleShape eye(2.5f);
		eye.setFillColor({20, 20, 20});
		eye.setPosition({(float)w * 0.66f, (float)h * 0.34f});
		rt.draw(eye);
	}
	else
	{
		sf::CircleShape b((float)h * 0.34f);
		b.setFillColor(body);
		b.setPosition({(float)w * 0.18f, (float)h * 0.30f});
		rt.draw(b);
		sf::CircleShape head((float)h * 0.22f);
		head.setFillColor(body);
		head.setPosition({(float)w * 0.60f, (float)h * 0.34f});
		rt.draw(head);
		for (float ox : {0.62f, 0.74f})
		{
			sf::RectangleShape ear({(float)w * 0.07f, (float)h * 0.30f});
			ear.setFillColor(darker(body, 20));
			ear.setPosition({(float)w * ox, (float)h * 0.02f});
			rt.draw(ear);
		}
		sf::CircleShape eye(2.5f);
		eye.setFillColor({20, 20, 20});
		eye.setPosition({(float)w * 0.80f, (float)h * 0.42f});
		rt.draw(eye);
	}
	rt.display();
	return rt.getTexture();
}

sf::Texture ResourceManager::makeTrafficTexture(sf::Color lamp)
{
	const float W = (float)Constants::TRAFFIC_WIDTH;
	const float H = (float)Constants::TRAFFIC_HEIGHT;
	sf::RenderTexture rt;
	(void)rt.resize({Constants::TRAFFIC_WIDTH, Constants::TRAFFIC_HEIGHT});
	rt.clear(sf::Color::Transparent);

	// dark housing
	sf::RectangleShape box({W * 0.72f, H * 0.92f});
	box.setFillColor({26, 26, 30});
	box.setOutlineColor({8, 8, 10});
	box.setOutlineThickness(1.f);
	box.setPosition({W * 0.14f, H * 0.04f});
	rt.draw(box);

	// soft glow behind the lit lamp
	float r = W * 0.26f;
	sf::CircleShape glow(r * 1.5f);
	glow.setFillColor({lamp.r, lamp.g, lamp.b, 60});
	glow.setOrigin({r * 1.5f, r * 1.5f});
	glow.setPosition({W * 0.5f, H * 0.5f});
	rt.draw(glow);

	// the lit lamp itself
	sf::CircleShape lampShape(r);
	lampShape.setFillColor(lamp);
	lampShape.setOrigin({r, r});
	lampShape.setPosition({W * 0.5f, H * 0.5f});
	rt.draw(lampShape);

	rt.display();
	return rt.getTexture();
}

// ── Builders ───────────────────────────────────────────────────────────────

sf::Texture ResourceManager::buildObjectTexture(const Constants::ObjectType& type,
	const Constants::ObjectColor& color)
{
	sf::Color sfColor = toSFColor(color);
	switch (type)
	{
	case Constants::ObjectType::Car:   return makeVehicleTexture(60, 36, sfColor, false);
	case Constants::ObjectType::Truck: return makeVehicleTexture(88, 42, sfColor, true);
	case Constants::ObjectType::Bird:  return makeAnimalTexture(46, 34, sfColor, true);
	case Constants::ObjectType::Rabit: return makeAnimalTexture(44, 40, sfColor, false);
	default:                           return makeTrafficTexture(toTrafficColor(Constants::TrafficLightColor::Green));
	}
}

sf::Texture ResourceManager::buildLaneTexture(const Constants::LaneType& type, int direction)
{
	if (type == Constants::LaneType::Road)
		return makeRoadTexture(direction == 1 ? sf::Color{44, 50, 56}
		                                      : sf::Color{38, 46, 42},
		                       direction);
	return makeGrassTexture({58, 150, 55});
}

// ── Cached lookups ───────────────────────────────────────────────────────────

const sf::Texture& ResourceManager::getObjectTexture(const Constants::ObjectType& type,
	const Constants::ObjectColor& color)
{
	// keyed by (type, color); std::map nodes are stable so the reference lasts
	static std::map<std::pair<int, int>, sf::Texture> cache;
	std::pair<int, int> key{static_cast<int>(type), static_cast<int>(color)};
	auto it = cache.find(key);
	if (it == cache.end())
		it = cache.emplace(key, buildObjectTexture(type, color)).first;
	return it->second;
}

const sf::Texture& ResourceManager::getLaneTexture(const Constants::LaneType& type, int direction)
{
	// grass looks the same in both directions, so collapse its key
	static std::map<std::pair<int, int>, sf::Texture> cache;
	int dir_key = (type == Constants::LaneType::Road) ? (direction == 1 ? 1 : -1) : 0;
	std::pair<int, int> key{static_cast<int>(type), dir_key};
	auto it = cache.find(key);
	if (it == cache.end())
		it = cache.emplace(key, buildLaneTexture(type, dir_key)).first;
	return it->second;
}

const sf::Texture& ResourceManager::getTrafficTexture(const Constants::TrafficLightColor& color)
{
	// one cached texture per light colour (Red / Yellow / Green)
	static std::map<int, sf::Texture> cache;
	int key = static_cast<int>(color);
	auto it = cache.find(key);
	if (it == cache.end())
		it = cache.emplace(key, makeTrafficTexture(toTrafficColor(color))).first;
	return it->second;
}
