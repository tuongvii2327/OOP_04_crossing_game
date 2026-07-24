#include "utils/resource_manager.h"
#include <algorithm>
#include <iostream>

#pragma region DEMO PROCEDURAL DRAWING (FALLBACK)

// Hàm phụ trợ tạo màu (Không dùng std::clamp để tránh lỗi trên compiler C++ cũ)
static sf::Color shade(sf::Color base, float factor)
{
    auto scaleChannel = [factor](std::uint8_t channel) -> std::uint8_t
    {
        int scaled = static_cast<int>(static_cast<float>(channel) * factor);
        if (scaled < 0)
            scaled = 0;
        if (scaled > 255)
            scaled = 255;
        return static_cast<std::uint8_t>(scaled);
    };
    return sf::Color(scaleChannel(base.r), scaleChannel(base.g), scaleChannel(base.b), base.a);
}

static sf::Color toSFColor(const Constants::ObjectColor &color)
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

static sf::Color toTrafficColor(const Constants::TrafficLightColor &color)
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

static sf::Texture makeRoadTexture(sf::Color asphalt, int direction)
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

static sf::Texture makeGrassTexture(sf::Color base)
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
            sf::CircleShape tuft(4.f, 3);
            tuft.setFillColor((row % 2 == 0) ? tuftDark : tuftLight);
            tuft.setPosition({x, y});
            rt.draw(tuft);
        }
        ++row;
    }

    rt.display();
    return rt.getTexture();
}

static sf::Texture makeVehicleTexture(unsigned w, unsigned h, sf::Color body, bool isTruck)
{
    sf::RenderTexture rt;
    (void)rt.resize({w, h});
    rt.clear(sf::Color::Transparent);

    float fw = static_cast<float>(w);
    float fh = static_cast<float>(h);
    sf::Color darkBody = shade(body, 0.75f);
    sf::Color windowColor(180, 220, 235, 235);
    sf::Color wheelColor(25, 25, 28);

    sf::RectangleShape chassis({fw, fh * 0.55f});
    chassis.setPosition({0.f, fh * 0.35f});
    chassis.setFillColor(body);
    rt.draw(chassis);

    if (isTruck)
    {
        sf::RectangleShape cargo({fw * 0.62f, fh * 0.8f});
        cargo.setPosition({0.f, fh * 0.05f});
        cargo.setFillColor(body);
        rt.draw(cargo);

        float cabWidth = fw * 0.3f;
        sf::RectangleShape cab({cabWidth, fh * 0.6f});
        cab.setPosition({fw - cabWidth, fh * 0.15f});
        cab.setFillColor(darkBody);
        rt.draw(cab);

        sf::RectangleShape window({cabWidth * 0.55f, fh * 0.28f});
        window.setPosition({fw - cabWidth * 0.85f, fh * 0.22f});
        window.setFillColor(windowColor);
        rt.draw(window);
    }
    else
    {
        sf::RectangleShape roof({fw * 0.5f, fh * 0.4f});
        roof.setPosition({fw * 0.27f, 0.f});
        roof.setFillColor(darkBody);
        rt.draw(roof);

        sf::RectangleShape window({fw * 0.4f, fh * 0.22f});
        window.setPosition({fw * 0.31f, fh * 0.05f});
        window.setFillColor(windowColor);
        rt.draw(window);
    }

    float wheelRadius = fh * 0.17f;
    auto drawWheelAt = [&](float centerX)
    {
        sf::CircleShape wheel(wheelRadius);
        wheel.setFillColor(wheelColor);
        wheel.setPosition({centerX - wheelRadius, fh - wheelRadius * 1.4f});
        rt.draw(wheel);
    };
    drawWheelAt(fw * 0.22f);
    drawWheelAt(fw * 0.78f);

    rt.display();
    return rt.getTexture();
}

static sf::Texture makeBushTexture(unsigned w, unsigned h, sf::Color body)
{
    sf::RenderTexture rt;
    (void)rt.resize({w, h});
    rt.clear(sf::Color::Transparent);

    float fw = static_cast<float>(w);
    float fh = static_cast<float>(h);
    sf::Color darker = shade(body, 0.75f);
    sf::Color lighter = shade(body, 1.2f);

    struct Lump
    {
        float cx, cy, r;
    };
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

static sf::Texture makeTrafficTexture(Constants::TrafficLightColor active)
{
    unsigned w = Constants::TRAFFIC_WIDTH;
    unsigned h = Constants::TRAFFIC_HEIGHT;
    sf::RenderTexture rt;
    (void)rt.resize({w, h});
    rt.clear(sf::Color::Transparent);

    float fw = static_cast<float>(w);
    float fh = static_cast<float>(h);

    sf::Color poleColor(150, 155, 160);
    float poleHeight = fh * 0.28f;
    float housingSize = fh;

    sf::RectangleShape pole({fw - housingSize, poleHeight});
    pole.setPosition({0.f, (fh - poleHeight) * 0.5f});
    pole.setFillColor(poleColor);
    rt.draw(pole);

    sf::RectangleShape endCap({poleHeight * 0.6f, poleHeight * 1.6f});
    endCap.setPosition({0.f, (fh - endCap.getSize().y) * 0.5f});
    endCap.setFillColor(poleColor);
    rt.draw(endCap);

    sf::RectangleShape housing({housingSize, housingSize});
    housing.setPosition({fw - housingSize, 0.f});
    housing.setFillColor(sf::Color(35, 35, 40));
    rt.draw(housing);

    float radius = housingSize * 0.32f;
    sf::CircleShape lamp(radius);
    lamp.setFillColor(toTrafficColor(active));
    lamp.setPosition({fw - housingSize * 0.5f - radius, housingSize * 0.5f - radius});
    rt.draw(lamp);

    rt.display();
    return rt.getTexture();
}
#pragma endregion

// ==========================================
// CLASS IMPLEMENTATION
// ==========================================

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
    (void)color;

    switch (type)
    {
    case Constants::DynamicObjectType::Car:
        return getInstance().getTexture("assets/textures/car_1.png");
    case Constants::DynamicObjectType::Truck:
        return getInstance().getTexture("assets/textures/truck_1.png");
    case Constants::DynamicObjectType::Bird:
        return getInstance().getTexture("assets/textures/bird_1.png");
    case Constants::DynamicObjectType::Rabbit:
        return getInstance().getTexture("assets/textures/rabbit_1.png");
    case Constants::DynamicObjectType::People:
    default:
        return getInstance().getTexture("assets/textures/player.png");
    }
}

const sf::Texture &ResourceManager::getStaticObjectTexture(const Constants::StaticObjectType &type, const Constants::ObjectColor &color)
{
    static std::map<std::pair<int, int>, sf::Texture> cache;
    std::pair<int, int> key{static_cast<int>(type), static_cast<int>(color)};

    if (cache.find(key) == cache.end())
    {
        switch (type)
        {
        case Constants::StaticObjectType::Rock:
            return getInstance().getTexture("assets/textures/rock.png"); // BẠN ĐÃ CÓ
        case Constants::StaticObjectType::Bush:
            cache.emplace(key, makeBushTexture(46, 46, sf::Color{60, 140, 60}));
            break;
        case Constants::StaticObjectType::Traffic:
        default:
            cache.emplace(key, makeTrafficTexture(Constants::TrafficLightColor::Green));
            break;
        }
    }
    return cache[key];
}

const sf::Texture &ResourceManager::getLaneTexture(const Constants::LaneType &type, int direction)
{
    static std::map<std::pair<int, int>, sf::Texture> cache;
    std::pair<int, int> key{static_cast<int>(type), direction};

    // Nếu chưa có trong cache thì tiến hành tự vẽ bằng code
    if (cache.find(key) == cache.end())
    {
        if (type == Constants::LaneType::Road)
        {
            // Vẽ đường nhựa
            cache.emplace(key, makeRoadTexture(sf::Color{44, 50, 56}, direction));
        }
        else
        {
            // Grass và Safe sẽ rơi vào đây: tự vẽ nền cỏ màu xanh sẫm với các cụm cỏ tam giác
            cache.emplace(key, makeGrassTexture(sf::Color{58, 150, 55}));
        }
    }

    return cache[key];
}

const sf::Texture &ResourceManager::getTrafficTexture(const Constants::TrafficLightColor &color)
{
    static std::map<int, sf::Texture> cache;
    int key = static_cast<int>(color);
    if (cache.find(key) == cache.end())
    {
        cache.emplace(key, makeTrafficTexture(color));
    }
    return cache[key];
}

void ResourceManager::clearAll()
{
    textures_.clear();
    fonts_.clear();
    sound_buffers_.clear();
}