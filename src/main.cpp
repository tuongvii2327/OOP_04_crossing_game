#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#include "../include/entities/animal.h"
#include "../include/entities/bird.h"
#include "../include/entities/car.h"
#include "../include/entities/dynamic_object.h"
#include "../include/entities/rabit.h"
#include "../include/entities/traffic_light.h"
#include "../include/entities/truck.h"
#include "../include/entities/vehicle.h"
#include "../include/utils/constants.h"
#include "../include/world/lane.h"

// ── RNG ────────────────────────────────────────────────────────────────────

static std::mt19937 g_rng{std::random_device{}()};

// ── Procedural textures ────────────────────────────────────────────────────

static sf::Texture makeRoadTexture(sf::Color asphalt, int direction)
{
	sf::RenderTexture rt;
	(void)rt.resize({Constants::LANE_WIDTH, Constants::LANE_HEIGHT});
	rt.clear(asphalt);

	// edge highlight
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

	// small direction arrow painted on the road
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

static sf::Texture makeGrassTexture(sf::Color base)
{
	sf::RenderTexture rt;
	(void)rt.resize({Constants::LANE_WIDTH, Constants::LANE_HEIGHT});
	rt.clear(base);

	// subtle stripe
	sf::RectangleShape stripe({(float)Constants::LANE_WIDTH, 3.f});
	stripe.setFillColor({0, 0, 0, 30});
	stripe.setPosition({0.f, Constants::LANE_HEIGHT * 0.5f - 1.5f});
	rt.draw(stripe);

	// random blades
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

static sf::Texture makeVehicleTexture(unsigned w, unsigned h,
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
		// cargo
		sf::RectangleShape cargo({(float)w * 0.65f, (float)h});
		cargo.setFillColor(darker(body, 35));
		rt.draw(cargo);
		// cab
		sf::RectangleShape cab({(float)w * 0.35f, (float)h});
		cab.setFillColor(body);
		cab.setPosition({(float)w * 0.65f, 0.f});
		rt.draw(cab);
		// cab window
		sf::RectangleShape win({(float)w * 0.14f, (float)h * 0.42f});
		win.setFillColor({160, 215, 245, 210});
		win.setPosition({(float)w * 0.68f, (float)h * 0.10f});
		rt.draw(win);
		// cargo detail lines
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
		// body
		sf::RectangleShape b({(float)w, (float)h});
		b.setFillColor(body);
		rt.draw(b);
		// roof highlight
		sf::RectangleShape roof({(float)w * 0.55f, (float)h * 0.38f});
		roof.setFillColor(darker(body, 20));
		roof.setPosition({(float)w * 0.22f, (float)h * 0.12f});
		rt.draw(roof);
		// front window
		sf::RectangleShape win({(float)w * 0.22f, (float)h * 0.36f});
		win.setFillColor({160, 215, 245, 210});
		win.setPosition({(float)w * 0.08f, (float)h * 0.13f});
		rt.draw(win);
		// rear window
		win.setPosition({(float)w * 0.70f, (float)h * 0.13f});
		rt.draw(win);
		// bottom shadow
		sf::RectangleShape shadow({(float)w, 5.f});
		shadow.setFillColor({0, 0, 0, 70});
		shadow.setPosition({0.f, (float)h - 5.f});
		rt.draw(shadow);
	}
	rt.display();
	return rt.getTexture();
}

// Bird / Rabit sprite (tests the Animal hierarchy visually).
static sf::Texture makeAnimalTexture(unsigned w, unsigned h,
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
		// round body
		sf::CircleShape b((float)h * 0.40f);
		b.setFillColor(body);
		b.setPosition({(float)w * 0.20f, (float)h * 0.18f});
		rt.draw(b);
		// wing
		sf::CircleShape wing((float)h * 0.24f);
		wing.setFillColor(darker(body, 30));
		wing.setPosition({(float)w * 0.32f, (float)h * 0.34f});
		rt.draw(wing);
		// beak
		sf::ConvexShape beak(3);
		beak.setPoint(0, {(float)w * 0.78f, (float)h * 0.40f});
		beak.setPoint(1, {(float)w * 0.98f, (float)h * 0.50f});
		beak.setPoint(2, {(float)w * 0.78f, (float)h * 0.58f});
		beak.setFillColor({240, 170, 40});
		rt.draw(beak);
		// eye
		sf::CircleShape eye(2.5f);
		eye.setFillColor({20, 20, 20});
		eye.setPosition({(float)w * 0.66f, (float)h * 0.34f});
		rt.draw(eye);
	}
	else
	{
		// body
		sf::CircleShape b((float)h * 0.34f);
		b.setFillColor(body);
		b.setPosition({(float)w * 0.18f, (float)h * 0.30f});
		rt.draw(b);
		// head
		sf::CircleShape head((float)h * 0.22f);
		head.setFillColor(body);
		head.setPosition({(float)w * 0.60f, (float)h * 0.34f});
		rt.draw(head);
		// ears
		for (float ox : {0.62f, 0.74f})
		{
			sf::RectangleShape ear({(float)w * 0.07f, (float)h * 0.30f});
			ear.setFillColor(darker(body, 20));
			ear.setPosition({(float)w * ox, (float)h * 0.02f});
			rt.draw(ear);
		}
		// eye
		sf::CircleShape eye(2.5f);
		eye.setFillColor({20, 20, 20});
		eye.setPosition({(float)w * 0.80f, (float)h * 0.42f});
		rt.draw(eye);
	}
	rt.display();
	return rt.getTexture();
}

static sf::Texture makeTrafficTexture()
{
	sf::RenderTexture rt;
	(void)rt.resize({Constants::TRAFFIC_WIDTH, Constants::TRAFFIC_HEIGHT});
	rt.clear({22, 22, 22});
	sf::RectangleShape box({
		Constants::TRAFFIC_WIDTH  * 0.76f,
		Constants::TRAFFIC_HEIGHT * 0.76f});
	box.setFillColor({48, 48, 48});
	box.setPosition({
		Constants::TRAFFIC_WIDTH  * 0.12f,
		Constants::TRAFFIC_HEIGHT * 0.12f});
	rt.draw(box);
	rt.display();
	return rt.getTexture();
}

// ── Lane configuration ─────────────────────────────────────────────────────

enum class LaneKind { StartGrass, SafeGrass, FinishGrass, Road, AnimalLane };

struct LaneConfig
{
	LaneKind  kind;
	int       direction; // 1 or -1, ignored for static grass
	bool      hasTraffic;
	bool      usesTrucks;
	bool      isBird;     // only for AnimalLane: true=Bird, false=Rabit
	sf::Color grassColor;
};

// 10 rows, index 0 = bottom (start), index 9 = top (finish)
// Every Road lane carries a mix of Car + Truck + Traffic; AnimalLane carries
// Bird/Rabit. Together they exercise the whole entity hierarchy on screen.
static const LaneConfig LANE_CFG[10] = {
	{ LaneKind::StartGrass,  1, false, false, false, {55, 140, 50}  }, // 0 start
	{ LaneKind::Road,        1, true,  false, false, {}             }, // 1 cars+trucks+traffic →
	{ LaneKind::Road,        1, true,  false, false, {}             }, // 2 cars+trucks+traffic →
	{ LaneKind::AnimalLane,  1, false, false, true,  {70, 130, 60}  }, // 3 birds →
	{ LaneKind::Road,        1, true,  false, false, {}             }, // 4 cars+trucks+traffic →
	{ LaneKind::Road,       -1, true,  false, false, {}             }, // 5 cars+trucks+traffic ←
	{ LaneKind::AnimalLane, -1, false, false, false, {68, 120, 58}  }, // 6 rabits ←
	{ LaneKind::Road,        1, true,  false, false, {}             }, // 7 cars+trucks+traffic →
	{ LaneKind::Road,       -1, true,  false, false, {}             }, // 8 cars+trucks+traffic ←
	{ LaneKind::FinishGrass, 1, false, false, false, {35, 160, 55}  }, // 9 finish
};

// speed level per lane (higher = harder; animal/grass lanes use 0)
static const int ROAD_LEVEL[10] = { 0, 1, 1, 0, 2, 2, 0, 3, 3, 0 };

// ── Lane simulation data ────────────────────────────────────────────────────
// Holds texture ownership and spawn timing.
// Object ownership (Vehicle/Animal/Traffic) is managed by Lane.
// Lightweight collision/animal lists are kept per lane for hit-testing and
// spawn spacing because Lane::list_obj is private.

struct LaneSim
{
	sf::Texture           laneTex;
	sf::Texture           carTex;       // Road lanes: car sprite
	sf::Texture           truckTex;     // Road lanes: truck sprite
	sf::Texture           objTex;       // AnimalLane: bird/rabit sprite
	Lane                 *lane          = nullptr;
	std::vector<Vehicle*> collisionList; // Road lanes
	std::vector<Animal*>  animalList;    // AnimalLane lanes
	std::vector<float>    spawnTimes;
	LaneKind              kind          = LaneKind::Road;
	float                 spawnCooldown = 0.f;
	bool                  isBird        = false;
};

// Minimum edge-to-edge gap enforced between a freshly spawned vehicle and the
// nearest existing one in the same lane.
constexpr float SPAWN_GAP = 4.f * (float)Constants::VISUAL_DISTANCE;

static const sf::Color VEHICLE_COLORS[] = {
	{210, 55,  55},   // red
	{50,  120, 215},  // blue
	{230, 195, 25},   // yellow
	{55,  175, 70},   // green
	{175, 55,  175},  // purple
	{215, 115, 35},   // orange
	{20,  180, 180},  // teal
};
static constexpr int NUM_COLORS = 7;

// Nudge a dir=-1 object one pixel inside the right edge so isOutOfMap()
// doesn't immediately discard a freshly spawned object.
static void clampSpawnEdge(LaneSim &sim, DynamicObject *o)
{
	if (sim.lane->getDirection() == -1)
	{
		auto p = o->getPosition();
		p.x = (float)Constants::WINDOW_WIDTH - 1.f;
		o->setPosition(p);
	}
}

// Spawn a randomly chosen Car or Truck (tests Car/Truck → Vehicle → DynamicObject).
static Vehicle *spawnVehicle(LaneSim &sim)
{
	Vehicle *v;
	if (g_rng() % 2 == 0)
		v = new Car(sim.carTex,
			static_cast<Constants::CarType>(g_rng() % 3), sim.lane);
	else
		v = new Truck(sim.truckTex,
			static_cast<Constants::TruckType>(g_rng() % 3), sim.lane);

	sim.lane->addObject(v);
	clampSpawnEdge(sim, v);
	sim.collisionList.push_back(v);
	return v;
}

// Spawn a Bird or Rabit (tests Bird/Rabit → Animal → DynamicObject).
static Animal *spawnAnimal(LaneSim &sim)
{
	Animal *a;
	if (sim.isBird) a = new Bird(sim.objTex, sim.lane);
	else            a = new Rabit(sim.objTex, sim.lane);

	sim.lane->addObject(a);
	clampSpawnEdge(sim, a);
	sim.animalList.push_back(a);
	return a;
}

// Delete out-of-map objects and drop them from the tracking list + the Lane.
template <class T>
static void cleanupInactive(LaneSim &sim, std::vector<T*> &objs)
{
	for (auto it = objs.begin(); it != objs.end(); )
	{
		if (!(*it)->isActive())
		{
			sim.lane->removeObject(*it);
			delete *it;
			it = objs.erase(it);
		}
		else
		{
			++it;
		}
	}
}

// True if the nearest existing object leaves at least `gap` edge-to-edge room
// at the spawn edge for a new object.
template <class T>
static bool spawnEdgeClear(const LaneSim &sim, const std::vector<T*> &objs,
	float laneX, float gap)
{
	int dir = sim.lane->getDirection();
	for (auto *o : objs)
	{
		if (dir == 1)
		{
			// new object's leading edge sits at laneX; nearest obj is at o.x
			if (o->getPosition().x < laneX + gap)
				return false;
		}
		else
		{
			// new object's leading edge sits at WINDOW_WIDTH; nearest obj right edge
			if (o->getPosition().x + o->getSprite().getLocalBounds().size.x
				> Constants::WINDOW_WIDTH - gap)
				return false;
		}
	}
	return true;
}

// ── Player ─────────────────────────────────────────────────────────────────

struct Player
{
	sf::Vector2f  pos;
	sf::FloatRect hitbox;
	int           laneIdx      = 0;
	bool          alive        = true;
	float         deathTimer   = 0.f;
	float         moveCooldown = 0.f;
	int           score        = 0;
	int           bestScore    = 0;
};

constexpr float PLAYER_SIZE   = 62.f;
constexpr float PLAYER_MARGIN = 9.f;

static sf::FloatRect calcPlayerHitbox(sf::Vector2f pos)
{
	return sf::FloatRect(
		{pos.x + PLAYER_MARGIN, pos.y + PLAYER_MARGIN},
		{PLAYER_SIZE - 2 * PLAYER_MARGIN, PLAYER_SIZE - 2 * PLAYER_MARGIN});
}

static sf::Vector2f laneCenter(int idx, float laneX)
{
	// lane idx 0 is at bottom of screen
	float laneY = (float)(9 - idx) * Constants::LANE_HEIGHT;
	return {
		laneX + Constants::LANE_WIDTH * 0.5f - PLAYER_SIZE * 0.5f,
		laneY + (Constants::LANE_HEIGHT - PLAYER_SIZE) * 0.5f
	};
}

// ── Draw player ────────────────────────────────────────────────────────────

static void drawPlayer(sf::RenderWindow &window, const Player &p,
	float laneX, float totalTime)
{
	if (!p.alive && p.deathTimer <= 0.f) return;

	sf::Color bodyCol = p.alive ? sf::Color{60, 200, 70} : sf::Color{220, 70, 70};

	// subtle bounce when alive
	float bobY = p.alive ? std::sin(totalTime * 6.f) * 2.5f : 0.f;
	sf::Vector2f base = {p.pos.x, p.pos.y + bobY};

	// drop shadow
	sf::RectangleShape shadow({PLAYER_SIZE - 12.f, 8.f});
	shadow.setFillColor({0, 0, 0, 80});
	shadow.setPosition({base.x + 6.f, base.y + PLAYER_SIZE - 6.f});
	window.draw(shadow);

	// body
	sf::RectangleShape body({PLAYER_SIZE - 10.f, PLAYER_SIZE - 12.f});
	body.setFillColor(bodyCol);
	body.setPosition({base.x + 5.f, base.y + 6.f});
	window.draw(body);

	// belly
	sf::RectangleShape belly({PLAYER_SIZE - 24.f, PLAYER_SIZE - 28.f});
	belly.setFillColor({
		(uint8_t)std::min(255, bodyCol.r + 55),
		(uint8_t)std::min(255, bodyCol.g + 35),
		(uint8_t)std::min(255, bodyCol.b + 20)});
	belly.setPosition({base.x + 12.f, base.y + 14.f});
	window.draw(belly);

	if (p.alive)
	{
		// eyes
		auto drawEye = [&](float ox, float oy)
		{
			sf::CircleShape white(6.f);
			white.setFillColor({245, 245, 245});
			white.setPosition({base.x + ox, base.y + oy});
			window.draw(white);
			sf::CircleShape pupil(3.f);
			pupil.setFillColor({20, 20, 20});
			pupil.setPosition({base.x + ox + 3.f, base.y + oy + 3.f});
			window.draw(pupil);
		};
		drawEye(12.f, 9.f);
		drawEye(PLAYER_SIZE - 28.f, 9.f);
	}
	else
	{
		// X eyes
		auto drawX = [&](float cx, float cy)
		{
			for (int d : {-1, 1})
			{
				sf::RectangleShape line({14.f, 3.f});
				line.setFillColor({240, 240, 240});
				line.setOrigin({7.f, 1.5f});
				line.setRotation(sf::degrees(45.f * d));
				line.setPosition({base.x + cx, base.y + cy});
				window.draw(line);
			}
		};
		drawX(18.f, 15.f);
		drawX(PLAYER_SIZE - 18.f, 15.f);

		// expanding death ring
		if (p.deathTimer > 0.f)
		{
			float frac = 1.f - p.deathTimer / 1.8f;
			float radius = PLAYER_SIZE * 0.5f + frac * PLAYER_SIZE * 1.2f;
			sf::CircleShape ring(radius);
			ring.setFillColor(sf::Color::Transparent);
			ring.setOutlineColor({220, 70, 70, (uint8_t)(200 * p.deathTimer / 1.8f)});
			ring.setOutlineThickness(3.f);
			ring.setOrigin({radius, radius});
			ring.setPosition({base.x + PLAYER_SIZE * 0.5f,
				base.y + PLAYER_SIZE * 0.5f});
			window.draw(ring);
		}
	}
}

// ── Sidebar ────────────────────────────────────────────────────────────────

static void drawSidebar(sf::RenderWindow &window, const std::vector<LaneSim> &sims,
	const Player &player, float fps, bool paused,
	float laneX, int numRows)
{
	// background
	sf::RectangleShape bg({laneX, (float)Constants::WINDOW_HEIGHT});
	bg.setFillColor({14, 14, 20});
	window.draw(bg);

	// per-lane strip + arrow
	for (int i = 0; i < numRows; ++i)
	{
		float rowY = (float)(numRows - 1 - i) * Constants::LANE_HEIGHT;
		float cy   = rowY + Constants::LANE_HEIGHT * 0.5f;
		const LaneSim &s = sims[i];

		sf::Color stripCol;
		if (s.kind == LaneKind::StartGrass || s.kind == LaneKind::FinishGrass)
			stripCol = {55, 160, 50, 220};
		else if (s.kind == LaneKind::SafeGrass)
			stripCol = {45, 130, 42, 180};
		else if (s.kind == LaneKind::AnimalLane)
			stripCol = {150, 110, 60, 210};
		else
			stripCol = (s.lane->getDirection() == 1)
				? sf::Color{40, 90, 210, 210}
				: sf::Color{200, 50, 50, 210};

		sf::RectangleShape strip({5.f, (float)Constants::LANE_HEIGHT - 6.f});
		strip.setFillColor(stripCol);
		strip.setPosition({3.f, rowY + 3.f});
		window.draw(strip);

		// highlight current player lane
		if (i == player.laneIdx)
		{
			sf::RectangleShape hl({laneX - 2.f, (float)Constants::LANE_HEIGHT - 2.f});
			hl.setFillColor({255, 255, 255, 12});
			hl.setPosition({1.f, rowY + 1.f});
			window.draw(hl);
		}

		// movement-direction arrow for moving lanes
		if (s.kind == LaneKind::Road || s.kind == LaneKind::AnimalLane)
		{
			int dir = s.lane->getDirection();
			sf::ConvexShape arr(3);
			if (dir == 1)
			{
				arr.setPoint(0, {20.f, cy - 6.f});
				arr.setPoint(1, {34.f, cy});
				arr.setPoint(2, {20.f, cy + 6.f});
			}
			else
			{
				arr.setPoint(0, {34.f, cy - 6.f});
				arr.setPoint(1, {20.f, cy});
				arr.setPoint(2, {34.f, cy + 6.f});
			}
			arr.setFillColor({210, 210, 210, 150});
			window.draw(arr);
		}
	}

	// score bar (right edge of sidebar)
	float barH = (float)Constants::WINDOW_HEIGHT - 30.f;
	sf::RectangleShape scoreBg({10.f, barH});
	scoreBg.setFillColor({30, 30, 40});
	scoreBg.setPosition({laneX - 16.f, 15.f});
	window.draw(scoreBg);

	float fillH = std::clamp(
		(float)player.score / (float)(numRows - 1) * barH,
		0.f, barH);
	sf::RectangleShape fill({10.f, fillH});
	fill.setFillColor({70, 210, 80});
	fill.setPosition({laneX - 16.f, 15.f + barH - fillH});
	window.draw(fill);

	// best score line
	float bestY = 15.f + barH - std::clamp(
		(float)player.bestScore / (float)(numRows - 1) * barH, 0.f, barH);
	sf::RectangleShape bestLine({10.f, 2.f});
	bestLine.setFillColor({240, 190, 30, 180});
	bestLine.setPosition({laneX - 16.f, bestY});
	window.draw(bestLine);

	// fps bar
	float fpsW = std::clamp(fps / 60.f, 0.f, 1.f) * (laneX - 26.f);
	sf::RectangleShape fpsBar({fpsW, 4.f});
	fpsBar.setFillColor(fps >= 50.f ? sf::Color{50, 220, 80} : sf::Color{220, 60, 50});
	fpsBar.setPosition({4.f, 4.f});
	window.draw(fpsBar);

	// pause dot
	if (paused)
	{
		sf::RectangleShape dot({8.f, 8.f});
		dot.setFillColor({240, 190, 30});
		dot.setPosition({4.f, 12.f});
		window.draw(dot);
	}
}

// ── Pause overlay ──────────────────────────────────────────────────────────

static void drawPauseOverlay(sf::RenderWindow &window)
{
	sf::RectangleShape dim({(float)Constants::WINDOW_WIDTH, (float)Constants::WINDOW_HEIGHT});
	dim.setFillColor({0, 0, 0, 115});
	window.draw(dim);

	float cx = Constants::WINDOW_WIDTH  * 0.5f;
	float cy = Constants::WINDOW_HEIGHT * 0.5f;
	for (float ox : {-14.f, 6.f})
	{
		sf::RectangleShape bar({10.f, 48.f});
		bar.setFillColor({240, 240, 240, 220});
		bar.setPosition({cx + ox, cy - 24.f});
		window.draw(bar);
	}
}

// ── Death screen ───────────────────────────────────────────────────────────

static void drawDeathOverlay(sf::RenderWindow &window, float deathTimer)
{
	if (deathTimer > 0.f) return; // still animating

	// dim
	sf::RectangleShape dim({(float)Constants::WINDOW_WIDTH, (float)Constants::WINDOW_HEIGHT});
	dim.setFillColor({0, 0, 0, 140});
	window.draw(dim);

	float cx = Constants::WINDOW_WIDTH  * 0.5f;
	float cy = Constants::WINDOW_HEIGHT * 0.5f;

	// skull X
	for (int d : {-1, 1})
	{
		sf::RectangleShape line({60.f, 8.f});
		line.setFillColor({220, 60, 60, 230});
		line.setOrigin({30.f, 4.f});
		line.setRotation(sf::degrees(45.f * d));
		line.setPosition({cx, cy - 20.f});
		window.draw(line);
	}

	// "press R" dots (3 dots below = R indicator)
	for (int i = 0; i < 3; ++i)
	{
		sf::CircleShape dot(5.f);
		dot.setFillColor({200, 200, 200, 200});
		dot.setPosition({cx - 15.f + i * 15.f, cy + 30.f});
		window.draw(dot);
	}
}

// ── Main ───────────────────────────────────────────────────────────────────

int main()
{
#ifdef _WIN32
	SetProcessDPIAware();
#endif

	sf::RenderWindow window(
		sf::VideoMode({Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT}),
		Constants::GAME_TITLE,
		sf::Style::Titlebar | sf::Style::Close);
	window.setFramerateLimit(60);

	constexpr int   NUM_ROWS = 10;
	constexpr float LANE_X   = (float)Constants::GAME_PLAY_BAR_WIDTH;

	// ── Build lanes ────────────────────────────────────────────────────────
	// Textures must outlive the lanes — stored inside LaneSim.
	// All Vehicle/Animal/Traffic pointers are owned by Lane.

	std::vector<LaneSim> sims(NUM_ROWS);
	sf::Texture trafficTex = makeTrafficTexture();

	for (int i = 0; i < NUM_ROWS; ++i)
	{
		LaneSim          &s   = sims[i];
		const LaneConfig &cfg = LANE_CFG[i];
		float laneY = (float)(NUM_ROWS - 1 - i) * Constants::LANE_HEIGHT;
		int   level = ROAD_LEVEL[i];

		s.kind   = cfg.kind;
		s.isBird = cfg.isBird;

		// ── Static grass lanes (start / safe / finish) ─────────────────────
		if (cfg.kind == LaneKind::StartGrass ||
			cfg.kind == LaneKind::SafeGrass  ||
			cfg.kind == LaneKind::FinishGrass)
		{
			s.laneTex = makeGrassTexture(cfg.grassColor);
			s.lane    = new Lane(s.laneTex, {LANE_X, laneY}, 1, 1);
			continue;
		}

		// ── Animal lanes (tests Bird / Rabit / Animal) ─────────────────────
		if (cfg.kind == LaneKind::AnimalLane)
		{
			s.laneTex = makeGrassTexture(cfg.grassColor);
			s.lane    = new Lane(s.laneTex, {LANE_X, laneY}, 1, cfg.direction);

			sf::Color body = cfg.isBird
				? sf::Color{235, 235, 245}   // bird: pale
				: sf::Color{180, 150, 120};  // rabit: brown
			s.objTex = cfg.isBird
				? makeAnimalTexture(46, 34, body, true)
				: makeAnimalTexture(44, 40, body, false);

			s.spawnCooldown = (float)(g_rng() % 200) / 100.f;

			// seed a couple of animals at random positions
			int seed = 2 + (int)(g_rng() % 2);
			for (int j = 0; j < seed; ++j)
			{
				Animal *a = spawnAnimal(s);
				float sw     = a->getSprite().getLocalBounds().size.x;
				float usable = Constants::LANE_WIDTH - sw - 40.f;
				if (usable > 0.f)
				{
					float rx = LANE_X + 20.f + (float)(g_rng() % (int)usable);
					auto  p  = a->getPosition(); p.x = rx;
					a->setPosition(p);
				}
			}
			continue;
		}

		// ── Road lanes (tests Car / Truck / Vehicle / Traffic) ─────────────
		s.laneTex = makeRoadTexture(
			cfg.direction == 1 ? sf::Color{44, 50, 56} : sf::Color{38, 46, 42},
			cfg.direction);

		// Each road lane gets its own car colour + a darker truck colour so the
		// mix of both vehicle types is easy to tell apart.
		sf::Color carBody   = VEHICLE_COLORS[i % NUM_COLORS];
		sf::Color truckBody = VEHICLE_COLORS[(i + 3) % NUM_COLORS];
		s.carTex   = makeVehicleTexture(60, 36, carBody,   false);
		s.truckTex = makeVehicleTexture(88, 42, truckBody, true);

		s.spawnCooldown = (float)(g_rng() % 280) / 100.f;
		s.lane = new Lane(s.laneTex, {LANE_X, laneY}, level, cfg.direction);

		// ── Traffic lights (tests Traffic) ─────────────────────────────────
		if (cfg.hasTraffic)
		{
			float lightY = laneY + (Constants::LANE_HEIGHT - Constants::TRAFFIC_HEIGHT) * 0.5f;
			int num_lights = g_rng() % 3 + 1; // 1 to 3
			float min_x = LANE_X + 0.15f * Constants::LANE_WIDTH;
			float max_x = LANE_X + 0.85f * Constants::LANE_WIDTH;
			float min_dist = 0.25f * Constants::LANE_WIDTH;

			std::vector<float> light_positions;
			for (int ti = 0; ti < num_lights; ++ti)
			{
				for (int attempt = 0; attempt < 15; ++attempt)
				{
					float lx = min_x + (float)(g_rng() % (int)(max_x - min_x));
					bool valid = true;
					for (float p : light_positions)
					{
						if (std::abs(p - lx) < min_dist)
						{
							valid = false;
							break;
						}
					}
					if (valid)
					{
						light_positions.push_back(lx);
						break;
					}
				}
			}

			for (float lx : light_positions)
			{
				float t = (float)(g_rng() % 500) / 100.f;
				auto col = (g_rng() % 2 == 0) ? Constants::TrafficLightColor::Green : Constants::TrafficLightColor::Red;
				auto *traf = new Traffic(trafficTex, s.lane, {lx, lightY}, col, t);
				s.lane->addTraffic(traf);
			}
		}

		// ── Seed initial vehicles ──────────────────────────────────────────
		// Lay them left-to-right with an edge-to-edge gap of at least SPAWN_GAP
		// so no two sprites overlap on the first frame.
		int   seed   = 1 + (int)(g_rng() % 3);
		float cursor = LANE_X + 20.f + (float)(g_rng() % 150);
		for (int j = 0; j < seed; ++j)
		{
			Vehicle *v  = spawnVehicle(s);
			float    sw = v->getSprite().getLocalBounds().size.x;
			if (cursor + sw > LANE_X + Constants::LANE_WIDTH - 20.f)
			{
				// no room left for this vehicle — discard it
				s.lane->removeObject(v);
				s.collisionList.pop_back();
				delete v;
				break;
			}
			auto p = v->getPosition(); p.x = cursor;
			v->setPosition(p);
			cursor += sw + SPAWN_GAP + (float)(g_rng() % 60);
		}
	}

	// ── Player setup ───────────────────────────────────────────────────────

	Player player;
	player.laneIdx = 0;
	player.pos     = laneCenter(0, LANE_X);
	player.hitbox  = calcPlayerHitbox(player.pos);

	// ── Clock / counters ───────────────────────────────────────────────────

	sf::Clock gameClock;
	sf::Clock fpsClock;
	float totalTime = 0.f;
	float fps       = 60.f;
	int   fpsCount  = 0;
	bool  paused    = false;

	// ── Game loop ──────────────────────────────────────────────────────────

	while (window.isOpen())
	{

		// ── Events ─────────────────────────────────────────────────────────
		while (auto ev = window.pollEvent())
		{
			if (ev->is<sf::Event::Closed>()) window.close();

			if (const auto *k = ev->getIf<sf::Event::KeyPressed>())
			{
				switch (k->code)
				{

				case sf::Keyboard::Key::Escape:
					window.close();
					break;

				case sf::Keyboard::Key::Space:
					paused = !paused;
					break;

				case sf::Keyboard::Key::Up:
				case sf::Keyboard::Key::W:
					if (player.alive && player.moveCooldown <= 0.f)
					{
						if (player.laneIdx < NUM_ROWS - 1)
						{
							player.laneIdx++;
							player.score = std::max(player.score, player.laneIdx);
							player.bestScore = std::max(player.bestScore, player.score);
						}
						player.pos          = laneCenter(player.laneIdx, LANE_X);
						player.hitbox       = calcPlayerHitbox(player.pos);
						player.moveCooldown = 0.16f;
					}
					break;

				case sf::Keyboard::Key::Down:
				case sf::Keyboard::Key::S:
					if (player.alive && player.moveCooldown <= 0.f)
					{
						if (player.laneIdx > 0) player.laneIdx--;
						player.pos          = laneCenter(player.laneIdx, LANE_X);
						player.hitbox       = calcPlayerHitbox(player.pos);
						player.moveCooldown = 0.16f;
					}
					break;

				case sf::Keyboard::Key::Left:
				case sf::Keyboard::Key::A:
					if (player.alive && player.moveCooldown <= 0.f)
					{
						player.pos.x = std::max(LANE_X,
							player.pos.x - Constants::CELL_SIZE);
						player.hitbox       = calcPlayerHitbox(player.pos);
						player.moveCooldown = 0.16f;
					}
					break;

				case sf::Keyboard::Key::Right:
				case sf::Keyboard::Key::D:
					if (player.alive && player.moveCooldown <= 0.f)
					{
						player.pos.x = std::min(
							player.pos.x + Constants::CELL_SIZE,
							LANE_X + Constants::LANE_WIDTH - PLAYER_SIZE);
						player.hitbox       = calcPlayerHitbox(player.pos);
						player.moveCooldown = 0.16f;
					}
					break;

				case sf::Keyboard::Key::R:
					if (!player.alive)
					{
						player.alive        = true;
						player.deathTimer   = 0.f;
						player.score        = 0;
						player.laneIdx      = 0;
						player.pos          = laneCenter(0, LANE_X);
						player.hitbox       = calcPlayerHitbox(player.pos);
					}
					break;

				default: break;
				}
			}
		}

		float dt = gameClock.restart().asSeconds();
		dt = std::min(dt, 0.05f) * (paused ? 0.f : 1.f);
		totalTime += dt;

		++fpsCount;
		if (float e = fpsClock.getElapsedTime().asSeconds(); e >= 0.5f)
		{
			fps = (float)fpsCount / e;
			fpsCount = 0;
			fpsClock.restart();
		}

		// ── Update ─────────────────────────────────────────────────────────

		if (!paused)
		{
			for (auto &s : sims)
			{
				bool isRoad   = (s.kind == LaneKind::Road);
				bool isAnimal = (s.kind == LaneKind::AnimalLane);
				if (!isRoad && !isAnimal) continue;

				// Lane::updateState() steps every DynamicObject (Vehicle or
				// Animal) and every Traffic, exercising the full hierarchy:
				//   Vehicle::updateState() / Animal::updateState()
				//   DynamicObject::isOutOfMap()/moveBy()/gapToObstacle()
				//   Lane::getNearest{Traffic,Vehicle,Animal}Ahead()
				//   Traffic::updateState() colour cycle
				s.lane->updateState(dt);

				// Remove out-of-map objects (isOutOfMap() already deactivated them)
				if (isRoad) cleanupInactive(s, s.collisionList);
				else        cleanupInactive(s, s.animalList);

				// Spawn rate limiting: at most 3 spawns per 20-second window
				float window_duration = 20.0f;
				size_t max_in_window = (s.kind == LaneKind::AnimalLane) ? 4 : 2;
				while (!s.spawnTimes.empty() &&
					totalTime - s.spawnTimes.front() > window_duration)
					s.spawnTimes.erase(s.spawnTimes.begin());

				bool can_spawn = false;
				if (s.spawnTimes.size() < max_in_window)
					can_spawn = isRoad
						? spawnEdgeClear(s, s.collisionList, LANE_X, SPAWN_GAP)
						: spawnEdgeClear(s, s.animalList, LANE_X, (float)Constants::VISUAL_DISTANCE);

				if (can_spawn)
				{
					if (isRoad) spawnVehicle(s);
					else        spawnAnimal(s);
					s.spawnTimes.push_back(totalTime);
				}
			}

			// player timers
			if (player.moveCooldown > 0.f) player.moveCooldown -= dt;
			if (!player.alive && player.deathTimer > 0.f) player.deathTimer -= dt;

			// Collision: player hitbox vs. Vehicle or Animal hitboxes
			if (player.alive)
			{
				LaneSim &ps = sims[player.laneIdx];
				auto testHit = [&](DynamicObject *o)
				{
					if (!o->isActive()) return false;
					return player.hitbox.findIntersection(o->getHitBox()).has_value();
				};
				if (ps.kind == LaneKind::Road)
				{
					for (auto *v : ps.collisionList)
						if (testHit(v)) { player.alive = false; player.deathTimer = 1.8f; break; }
				}
				else if (ps.kind == LaneKind::AnimalLane)
				{
					for (auto *a : ps.animalList)
						if (testHit(a)) { player.alive = false; player.deathTimer = 1.8f; break; }
				}
			}

			// win condition: reached finish grass
			if (player.alive && player.laneIdx == NUM_ROWS - 1)
			{
				player.score     = std::max(player.score, player.laneIdx);
				player.bestScore = std::max(player.bestScore, player.score);
				player.laneIdx   = 0;
				player.score     = 0;
				player.pos       = laneCenter(0, LANE_X);
				player.hitbox    = calcPlayerHitbox(player.pos);
			}
		}

		// ── Render ─────────────────────────────────────────────────────────

		window.clear({20, 22, 28});

		// Lane::render() draws the lane sprite + all DynamicObjects + Traffics
		for (auto &s : sims)
			s.lane->render(window);

		// lane separators
		for (int i = 0; i < NUM_ROWS - 1; ++i)
		{
			sf::RectangleShape sep({(float)Constants::WINDOW_WIDTH, 1.f});
			sep.setFillColor({60, 60, 70, 80});
			sep.setPosition({0.f, (float)(i + 1) * Constants::LANE_HEIGHT});
			window.draw(sep);
		}

		// finish line dashes on row 9
		{
			float finY = 0.f; // laneIdx 9 → screen Y = 0
			for (int x = 0; x < (int)Constants::LANE_WIDTH; x += 22)
			{
				sf::RectangleShape checker({10.f, 10.f});
				checker.setFillColor((x / 22) % 2 == 0
					? sf::Color{240, 240, 240, 90}
					: sf::Color{20, 20, 20, 60});
				checker.setPosition({LANE_X + (float)x, finY});
				window.draw(checker);
				checker.setPosition({LANE_X + (float)x, finY + Constants::LANE_HEIGHT - 10.f});
				window.draw(checker);
			}
		}

		// player
		drawPlayer(window, player, LANE_X, totalTime);

		// sidebar (drawn on top to mask vehicles that slide under it)
		drawSidebar(window, sims, player, fps, paused, LANE_X, NUM_ROWS);

		// overlays
		if (paused)
			drawPauseOverlay(window);

		if (!player.alive)
			drawDeathOverlay(window, player.deathTimer);

		window.display();
	}

	// Lane destructor deletes every owned DynamicObject and Traffic pointer
	for (auto &s : sims)
		delete s.lane;

	return 0;
}
