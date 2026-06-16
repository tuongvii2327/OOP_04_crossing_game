#include <SFML/Graphics.hpp>
#include <cassert>
#include <cmath>
#include <iostream>
#include <string>

#include "../include/entities/car.h"
#include "../include/entities/dynamic_object.h"
#include "../include/entities/traffic_light.h"
#include "../include/entities/truck.h"
#include "../include/entities/vehicle.h"
#include "../include/utils/constants.h"
#include "../include/world/lane.h"

// ── helpers ──────────────────────────────────────────────────────────────────

static int g_pass = 0, g_fail = 0;

static void check(bool cond, const std::string &name) {
  if (cond) {
    std::cout << "  [PASS] " << name << "\n";
    ++g_pass;
  } else {
    std::cout << "  [FAIL] " << name << "\n";
    ++g_fail;
  }
}

static bool nearlyEqual(float a, float b, float eps = 0.5f) {
  return std::abs(a - b) <= eps;
}

// ── texture factories (no external image files needed) ───────────────────────

static sf::Texture makeLaneTexture(unsigned w = Constants::LANE_WIDTH,
                                   unsigned h = Constants::LANE_HEIGHT) {
  sf::RenderTexture rt;
  (void)rt.resize(sf::Vector2u{w, h});
  rt.clear(sf::Color{60, 60, 60});
  sf::RectangleShape dash(sf::Vector2f{40.f, 4.f});
  dash.setFillColor(sf::Color{220, 220, 180});
  float yMid = h / 2.f - 2.f;
  for (unsigned x = 20; x + 40 < w; x += 80) {
    dash.setPosition(sf::Vector2f{(float)x, yMid});
    rt.draw(dash);
  }
  rt.display();
  return rt.getTexture();
}

static sf::Texture makeCarTexture(unsigned w = 60, unsigned h = 40,
                                  sf::Color body = sf::Color{220, 60, 60}) {
  sf::RenderTexture rt;
  (void)rt.resize(sf::Vector2u{w, h});
  rt.clear(sf::Color::Transparent);
  sf::RectangleShape rect(sf::Vector2f{(float)w, (float)h});
  rect.setFillColor(body);
  rt.draw(rect);
  rt.display();
  return rt.getTexture();
}

static sf::Texture makeTruckTexture(unsigned w = 90, unsigned h = 44,
                                    sf::Color body = sf::Color{60, 120, 200}) {
  sf::RenderTexture rt;
  (void)rt.resize(sf::Vector2u{w, h});
  rt.clear(sf::Color::Transparent);
  sf::RectangleShape rect(sf::Vector2f{(float)w, (float)h});
  rect.setFillColor(body);
  rt.draw(rect);
  rt.display();
  return rt.getTexture();
}

static sf::Texture makeTrafficTexture(
    unsigned w = Constants::TRAFFIC_WIDTH,
    unsigned h = Constants::TRAFFIC_HEIGHT,
    Constants::TrafficLightColor color = Constants::TrafficLightColor::Green) {
  sf::RenderTexture rt;
  (void)rt.resize(sf::Vector2u{w, h});
  rt.clear(sf::Color{30, 30, 30});
  sf::Color c;
  switch (color) {
  case Constants::TrafficLightColor::Red:    c = sf::Color{230, 40,  40};  break;
  case Constants::TrafficLightColor::Yellow: c = sf::Color{240, 200, 20};  break;
  default:                                   c = sf::Color{40,  210, 80};  break;
  }
  float r = std::min(w, h) / 2.f - 2.f;
  sf::CircleShape circle(r);
  circle.setFillColor(c);
  circle.setPosition(sf::Vector2f{(w - r * 2.f) / 2.f, (h - r * 2.f) / 2.f});
  rt.draw(circle);
  rt.display();
  return rt.getTexture();
}

// ── Group 1: DynamicObject basics ────────────────────────────────────────────
// DynamicObject is abstract; Car (the simplest concrete subclass) is used here.
static void testDynamicObject() {
  std::cout << "\n=== DynamicObject ===\n";

  sf::Texture laneT = makeLaneTexture();
  sf::Texture carT  = makeCarTexture();

  // ── 1a. No-lane construction ──────────────────────────────────────────────
  // When no lane is supplied calcDirection() returns 1.
  // calcSpawnPosition() returns (0,0) when current_lane==nullptr.
  Car noLaneCar(carT, Constants::CarType::RedCar, nullptr);
  check(noLaneCar.getDirection() == 1,      "no-lane: direction defaults to 1");
  check(nearlyEqual(noLaneCar.getPosition().x, 0.f),
                                             "no-lane: position.x == 0");
  check(noLaneCar.isActive(),                "no-lane: isActive() == true");
  check(noLaneCar.getLane() == nullptr,      "no-lane: getLane() == nullptr");

  // ── 1b. Lane-bound construction ───────────────────────────────────────────
  Lane laneRight(laneT, sf::Vector2f{140.f, 200.f}, 1, 1);
  Car carRight(carT, Constants::CarType::RedCar, &laneRight);
  check(carRight.getDirection() == 1,        "right lane: direction == 1");
  check(carRight.getLane() == &laneRight,    "right lane: getLane() correct");

  Lane laneLeft(laneT, sf::Vector2f{140.f, 300.f}, 1, -1);
  Car carLeft(carT, Constants::CarType::BlueCar, &laneLeft);
  check(carLeft.getDirection() == -1,        "left lane: direction == -1");

  // ── 1c. setDirection guards ───────────────────────────────────────────────
  carRight.setDirection(0);
  check(carRight.getDirection() == 1,        "setDirection(0) is ignored");
  carRight.setDirection(2);
  check(carRight.getDirection() == 1,        "setDirection(2) is ignored");
  carRight.setDirection(-1);
  check(carRight.getDirection() == -1,       "setDirection(-1) accepted");
  carRight.setDirection(1);
  check(carRight.getDirection() == 1,        "setDirection(1) accepted");

  // ── 1d. setPosition syncs hitbox ─────────────────────────────────────────
  sf::Vector2f newPos{300.f, 250.f};
  carRight.setPosition(newPos);
  sf::FloatRect hb = carRight.getHitBox();
  check(nearlyEqual(carRight.getPosition().x, 300.f),
                                             "setPosition: position.x updated");
  check(hb.position.x >= newPos.x,          "setPosition: hitbox.x >= new position.x");

  // ── 1e. isOutOfMap ────────────────────────────────────────────────────────
  carRight.setActiveState(true);
  carRight.setPosition(sf::Vector2f{-500.f, 200.f}); // far left
  check(carRight.isOutOfMap(),               "isOutOfMap: true when far left");

  carRight.setActiveState(true);
  carRight.setPosition(sf::Vector2f{Constants::WINDOW_WIDTH + 10.f, 200.f}); // far right
  check(carRight.isOutOfMap(),               "isOutOfMap: true when far right");

  // ── 1f. Hitbox size ratio ─────────────────────────────────────────────────
  // Reset to sensible position
  Lane laneFresh(laneT, sf::Vector2f{140.f, 400.f}, 1, 1);
  Car carFresh(carT, Constants::CarType::RedCar, &laneFresh);
  sf::FloatRect hbFresh = carFresh.getHitBox();
  float spriteW = carFresh.getSprite().getLocalBounds().size.x;
  check(nearlyEqual(hbFresh.size.x, spriteW * Constants::HITBOX_SIZE_RATIO, 1.f),
                                             "hitbox width == sprite_w * HITBOX_SIZE_RATIO");
  check(hbFresh.size.x > 0.f && hbFresh.size.y > 0.f,
                                             "hitbox has positive size");
}

// ── Group 2: Lane ─────────────────────────────────────────────────────────────
static void testLane() {
  std::cout << "\n=== Lane ===\n";

  sf::Texture laneT = makeLaneTexture();
  Lane lane(laneT, sf::Vector2f{140.f, 200.f}, 2, 1);

  check(lane.getLevel() == 2,                          "getLevel() == 2");
  check(lane.getDirection() == 1,                      "getDirection() == 1");
  check(nearlyEqual(lane.getPosition().x, 140.f),      "getPosition().x == 140");
  check(nearlyEqual(lane.getPosition().y, 200.f),      "getPosition().y == 200");

  // setLevel
  lane.setLevel(5);
  check(lane.getLevel() == 5,                          "setLevel(5)");

  // setDirection only accepts 1 or -1
  lane.setDirection(0);
  check(lane.getDirection() == 1,                      "setDirection(0) ignored");
  lane.setDirection(-1);
  check(lane.getDirection() == -1,                     "setDirection(-1)");
  lane.setDirection(99);
  check(lane.getDirection() == -1,                     "setDirection(99) ignored");
}

// ── Group 3: Traffic light ────────────────────────────────────────────────────
static void testTrafficLight() {
  std::cout << "\n=== TrafficLight (Traffic) ===\n";

  sf::Texture laneT = makeLaneTexture();
  sf::Texture trafT = makeTrafficTexture(
      Constants::TRAFFIC_WIDTH, Constants::TRAFFIC_HEIGHT,
      Constants::TrafficLightColor::Green);

  Lane lane(laneT, sf::Vector2f{140.f, 0.f}, 1, 1);

  // ── 3a. Initial state ─────────────────────────────────────────────────────
  Traffic traf(trafT, &lane, sf::Vector2f{400.f, 0.f},
               Constants::TrafficLightColor::Green, 0.f);
  check(traf.getColor() == Constants::TrafficLightColor::Green,
        "initial color Green");
  check(nearlyEqual(traf.getTimer(), 0.f, 0.01f),
        "initial timer == 0 when init_timer=0");
  check(traf.getGreenDuration() > 0.f,   "green_duration > 0");
  check(traf.getRedDuration() > 0.f,     "red_duration > 0");

  // ── 3b. init_timer propagation ────────────────────────────────────────────
  Traffic traf2(trafT, &lane, sf::Vector2f{500.f, 0.f},
                Constants::TrafficLightColor::Green, 2.5f);
  check(nearlyEqual(traf2.getTimer(), 2.5f, 0.01f),
        "init_timer=2.5 is preserved after construction");

  // ── 3c. Green → Yellow ────────────────────────────────────────────────────
  float green = traf.getGreenDuration();
  traf.updateState(green + 0.1f);
  check(traf.getColor() == Constants::TrafficLightColor::Yellow,
        "Green → Yellow after green_duration");

  // ── 3d. Yellow → Red ──────────────────────────────────────────────────────
  traf.updateState(Constants::YELLOW_DURATION + 0.1f);
  check(traf.getColor() == Constants::TrafficLightColor::Red,
        "Yellow → Red after YELLOW_DURATION");

  // ── 3e. Red → Green ───────────────────────────────────────────────────────
  float red = traf.getRedDuration();
  traf.updateState(red + 0.1f);
  check(traf.getColor() == Constants::TrafficLightColor::Green,
        "Red → Green after red_duration");

  // ── 3f. Timer resets on color transition ─────────────────────────────────
  check(nearlyEqual(traf.getTimer(), 0.f, 0.01f),
        "timer resets to 0 on color change");

  // ── 3g. setColor / setTimer independence ─────────────────────────────────
  traf.setTimer(99.f);
  check(nearlyEqual(traf.getTimer(), 99.f, 0.01f),
        "setTimer(99) sets timer directly");
}

// ── Group 4: Car ──────────────────────────────────────────────────────────────
static void testCar() {
  std::cout << "\n=== Car ===\n";

  sf::Texture laneT = makeLaneTexture();
  sf::Texture carT  = makeCarTexture(60, 40, sf::Color{40, 100, 220}); // blue

  Lane lane(laneT, sf::Vector2f{140.f, 200.f}, 1, 1);
  Car car(carT, Constants::CarType::BlueCar, &lane);

  check(car.getType() == Constants::DynamicObjectType::Car,
        "getType() == Car");
  check(car.getCarType() == Constants::CarType::BlueCar,
        "getCarType() == BlueCar");
  check(car.getDirection() == 1,                "direction follows lane (dir==1)");
  check(car.isActive(),                         "isActive() == true");

  // Speed in range [20, 35] for level 1
  float spd = car.getBaseSpeed();
  check(spd >= 20.f && spd <= 35.f,            "base_speed in [20, 35] at level 1");

  // safe_distance == base_speed * SAFE_DISTANCE_FACTOR
  check(nearlyEqual(car.getSafeDistance(),
                    spd * Constants::SAFE_DISTANCE_FACTOR, 0.1f),
        "safe_distance == base_speed * SAFE_DISTANCE_FACTOR");

  // Velocity: vx == base_speed, vy == 0 (direction==1)
  check(nearlyEqual(car.getVelocity().x, spd, 0.1f),
        "velocity.x == base_speed (direction==1)");
  check(nearlyEqual(car.getVelocity().y, 0.f), "velocity.y == 0");

  // Hitbox
  sf::FloatRect hb = car.getHitBox();
  check(hb.size.x > 0.f && hb.size.y > 0.f,   "hitbox has positive size");
  check(nearlyEqual(hb.size.x,
                    car.getSprite().getLocalBounds().size.x * Constants::HITBOX_SIZE_RATIO, 1.f),
        "hitbox width ≈ sprite_width * HITBOX_SIZE_RATIO");

  // Direction -1: velocity.x must be negative
  sf::Texture carTR = makeCarTexture(60, 40, sf::Color{220, 50, 50});
  Lane laneL(laneT, sf::Vector2f{140.f, 300.f}, 1, -1);
  Car carLeft(carTR, Constants::CarType::RedCar, &laneL);
  check(carLeft.getDirection() == -1,           "left-lane car: direction == -1");
  check(carLeft.getVelocity().x < 0.f,          "left-lane car: velocity.x < 0");

  // setCarType
  car.setCarType(Constants::CarType::YellowCar);
  check(car.getCarType() == Constants::CarType::YellowCar,
        "setCarType(YellowCar)");

  // Motion state defaults to Cruising
  check(car.getMotionState() == Constants::MotionState::Cruising,
        "motion_state defaults to Cruising");
  check(car.getLightDecision() == Constants::LightDecision::None,
        "light_decision defaults to None");
}

// ── Group 5: Truck ────────────────────────────────────────────────────────────
static void testTruck() {
  std::cout << "\n=== Truck ===\n";

  sf::Texture laneT  = makeLaneTexture();
  sf::Texture truckT = makeTruckTexture(90, 44, sf::Color{60, 120, 200});

  Lane lane(laneT, sf::Vector2f{140.f, 400.f}, 1, 1);
  Truck truck(truckT, Constants::TruckType::BlueTruck, &lane);

  check(truck.getType() == Constants::DynamicObjectType::Truck,
        "getType() == Truck");
  check(truck.getTruckType() == Constants::TruckType::BlueTruck,
        "getTruckType() == BlueTruck");

  // Speed in range [9, 11] for level 1
  float spd = truck.getBaseSpeed();
  check(spd >= 9.f && spd <= 11.f,             "base_speed in [9, 11] at level 1");

  // Direction inherits from lane
  check(truck.getDirection() == 1,             "direction follows lane");
  check(truck.getVelocity().x > 0.f,           "velocity.x > 0 for direction==1");

  // Truck with direction -1
  Lane laneLeft(laneT, sf::Vector2f{140.f, 500.f}, 1, -1);
  Truck truckLeft(truckT, Constants::TruckType::RedTruck, &laneLeft);
  check(truckLeft.getDirection() == -1,        "left-lane truck: direction == -1");
  check(truckLeft.getVelocity().x < 0.f,       "left-lane truck: velocity.x < 0");

  // setTruckType
  truck.setTruckType(Constants::TruckType::YellowTruck);
  check(truck.getTruckType() == Constants::TruckType::YellowTruck,
        "setTruckType(YellowTruck)");
}

// ── Group 6: Vehicle::updateState – no obstacle ──────────────────────────────
// NOTE: objects passed to lane.addObject() must be heap-allocated because
//       Lane::~Lane() calls delete on every pointer in its list.
static void testVehicleUpdateNoObstacle() {
  std::cout << "\n=== Vehicle::updateState – no obstacle ===\n";

  sf::Texture laneT = makeLaneTexture();
  sf::Texture carT  = makeCarTexture(60, 40, sf::Color{220, 80, 80});

  Lane lane(laneT, sf::Vector2f{140.f, 500.f}, 1, 1);

  // Heap-allocate so Lane::~Lane() can safely delete it
  Car *car = new Car(carT, Constants::CarType::RedCar, &lane);
  lane.addObject(car); // lane takes ownership

  float startX = car->getPosition().x;
  float speed  = car->getBaseSpeed();
  float dt     = 0.016f;

  car->updateState(dt);

  float expectedX = startX + speed * dt;
  check(nearlyEqual(car->getPosition().x, expectedX, 1.f),
        "car moves by speed*dt per frame with no obstacle");

  // Multiple frames: car keeps cruising at base_speed
  float x0 = car->getPosition().x;
  for (int i = 0; i < 5; ++i)
    car->updateState(dt);
  float xN = car->getPosition().x;
  check(xN > x0, "car continues moving right over multiple frames");
}

// ── Group 7: Vehicle brakes at red light ─────────────────────────────────────
static void testVehicleBrakesAtRedLight() {
  std::cout << "\n=== Vehicle::updateState – brakes at red light ===\n";

  sf::Texture laneT = makeLaneTexture();
  sf::Texture carT  = makeCarTexture(60, 40, sf::Color{220, 80, 80});
  sf::Texture trafT = makeTrafficTexture(
      Constants::TRAFFIC_WIDTH, Constants::TRAFFIC_HEIGHT,
      Constants::TrafficLightColor::Red);

  Lane lane(laneT, sf::Vector2f{140.f, 600.f}, 1, 1);

  // Heap-allocate; lane takes ownership of both
  Car *car = new Car(carT, Constants::CarType::RedCar, &lane);
  lane.addObject(car);

  // Place car at a known position inside the play area
  car->setPosition(sf::Vector2f{200.f, 600.f});

  float safeDist = car->getSafeDistance();
  float carRight = car->getPosition().x + car->getSprite().getLocalBounds().size.x;
  float lightX   = carRight + Constants::VISUAL_DISTANCE + safeDist * 0.5f;

  Traffic *red = new Traffic(trafT, &lane,
                              sf::Vector2f{lightX, 600.f},
                              Constants::TrafficLightColor::Red, 0.f);
  lane.addTraffic(red);

  float speedBefore = std::abs(car->getVelocity().x);

  for (int i = 0; i < 10; ++i)
    car->updateState(0.016f);

  float speedAfter = std::abs(car->getVelocity().x);
  check(speedAfter < speedBefore,
        "car decelerates when red light is within safe_distance");

  // After enough frames the car should be nearly stopped (or stopped)
  for (int i = 0; i < 60; ++i)
    car->updateState(0.016f);
  float speedFinal = std::abs(car->getVelocity().x);
  check(speedFinal < speedBefore * 0.5f,
        "car is significantly slowed after sustained red-light exposure");
}

// ── Group 8: Vehicle follows slower leader ────────────────────────────────────
static void testVehicleFollowsLeader() {
  std::cout << "\n=== Vehicle::updateState – follows slower leader ===\n";

  sf::Texture laneT    = makeLaneTexture();
  sf::Texture carTRed  = makeCarTexture(60, 40, sf::Color{220, 50, 50});
  sf::Texture carTBlue = makeCarTexture(60, 40, sf::Color{40, 100, 220});

  Lane lane(laneT, sf::Vector2f{140.f, 700.f}, 1, 1);

  // Heap-allocate; lane takes ownership
  Car *leader = new Car(carTRed, Constants::CarType::RedCar, &lane);
  lane.addObject(leader);
  leader->setPosition(sf::Vector2f{400.f, 700.f});

  Car *follower = new Car(carTBlue, Constants::CarType::BlueCar, &lane);
  lane.addObject(follower);

  // Place follower inside safe_distance behind the leader
  float safeDist   = follower->getSafeDistance();
  float followerX  = leader->getPosition().x
                   - follower->getSprite().getLocalBounds().size.x
                   - Constants::VISUAL_DISTANCE
                   - safeDist * 0.4f;
  follower->setPosition(sf::Vector2f{followerX, 700.f});

  // Force leader to move slower than follower
  leader->setVelocity(sf::Vector2f{10.f, 0.f});

  float speedBefore = follower->getVelocity().x;
  follower->updateState(0.016f);
  float speedAfter  = follower->getVelocity().x;

  check(speedAfter < speedBefore,
        "follower reduces speed when slower leader is ahead");
}

// ── Group 9: Lane addObject / addTraffic count via render (smoke) ─────────────
// Lane has no size() accessor, so we smoke-test by creating a window and
// verifying no crash occurs when rendering a lane that holds objects.
static void testLaneHoldsObjects() {
  std::cout << "\n=== Lane holds objects (smoke) ===\n";

  sf::Texture laneT  = makeLaneTexture();
  sf::Texture carT   = makeCarTexture();
  sf::Texture trafT  = makeTrafficTexture();

  Lane lane(laneT, sf::Vector2f{140.f, 0.f}, 1, 1);

  Car     *car  = new Car(carT, Constants::CarType::RedCar, &lane);
  Traffic *traf = new Traffic(trafT, &lane, sf::Vector2f{600.f, 0.f},
                               Constants::TrafficLightColor::Green, 0.f);
  lane.addObject(car);
  lane.addTraffic(traf);

  // addTraffic and addObject should not crash, and getNearestTrafficAhead
  // should return nullptr when the light is green and out of safe_distance
  Traffic *found = lane.getNearestTrafficAhead(*car);
  check(found == nullptr,
        "getNearestTrafficAhead returns nullptr for a green light");

  // Change to red – now it should be found if within safe_distance
  traf->setColor(Constants::TrafficLightColor::Red);
  float safeDist = car->getSafeDistance();
  float carRight = car->getPosition().x + car->getSprite().getLocalBounds().size.x;
  // Place traf just inside safe_distance from the car
  traf->setPosition(sf::Vector2f{carRight + Constants::VISUAL_DISTANCE + safeDist * 0.5f, 0.f});
  found = lane.getNearestTrafficAhead(*car);
  check(found != nullptr,
        "getNearestTrafficAhead finds a red light within safe_distance");
}

// ── main ─────────────────────────────────────────────────────────────────────
int main(int /*argc*/, char * /*argv*/[]) {
  std::cout << "========================================\n";
  std::cout << "  CrossyRoad – Logic Unit Tests\n";
  std::cout << "========================================\n";

  testDynamicObject();
  testLane();
  testTrafficLight();
  testCar();
  testTruck();
  testVehicleUpdateNoObstacle();
  testVehicleBrakesAtRedLight();
  testVehicleFollowsLeader();
  testLaneHoldsObjects();

  std::cout << "\n========================================\n";
  std::cout << "  Results: " << g_pass << " passed, " << g_fail << " failed\n";
  std::cout << "========================================\n";

  return (g_fail == 0) ? 0 : 1;
}
