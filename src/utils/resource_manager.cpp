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
    case Constants::ObjectColor::Red:
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

static sf::Texture makeRoadTexture(sf::Color asphalt)
{
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
    unsigned w = Constants::getSizeInGame(Constants::StaticObjectType::Traffic).width;
    unsigned h = Constants::getSizeInGame(Constants::StaticObjectType::Traffic).height;
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

static sf::Texture makeWoodTexture(unsigned w, unsigned h, sf::Color body)
{
    sf::RenderTexture rt;
    (void)rt.resize({w, h});
    rt.clear(sf::Color::Transparent);

    float fw = static_cast<float>(w);
    float fh = static_cast<float>(h);
    sf::Color darker = shade(body, 0.7f);
    sf::Color lighter = shade(body, 1.2f);

    // main body
    sf::RectangleShape main_body({fw, fh});
    main_body.setPosition({0.f, 0.f});
    main_body.setFillColor(body);
    rt.draw(main_body);

    // darker edges (top and bottom)
    sf::RectangleShape top_edge({fw, fh * 0.12f});
    top_edge.setPosition({0.f, 0.f});
    top_edge.setFillColor(darker);
    rt.draw(top_edge);

    sf::RectangleShape bottom_edge({fw, fh * 0.12f});
    bottom_edge.setPosition({0.f, fh * 0.88f});
    bottom_edge.setFillColor(darker);
    rt.draw(bottom_edge);

    // horizontal wood grain lines
    sf::Color grainColor = shade(body, 0.85f);
    for (float y = fh * 0.25f; y < fh * 0.85f; y += fh * 0.18f)
    {
        sf::RectangleShape grain({fw * 0.9f, 2.f});
        grain.setPosition({fw * 0.05f, y});
        grain.setFillColor(grainColor);
        rt.draw(grain);
    }

    // light highlight on top-left
    sf::RectangleShape highlight({fw * 0.3f, fh * 0.06f});
    highlight.setPosition({fw * 0.08f, fh * 0.15f});
    highlight.setFillColor(lighter);
    rt.draw(highlight);

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

const sf::Texture &ResourceManager::getDynamicObjectTexture(const Constants::DynamicObjectType &type, const Constants::ObjectColor &color, int cur_frame)
{
    std::string type_name, color_name, frame;
    cur_frame = (cur_frame / 4) % Constants::getNumberOfFrames(type) + 1;
    frame = std::to_string(cur_frame);
    switch (type)
    {
    case Constants::DynamicObjectType::Car:
        type_name = "car";
        break;
    case Constants::DynamicObjectType::Truck:
        type_name = "truck";
        break;
    case Constants::DynamicObjectType::Bird:
        type_name = "bird";
        break;
    case Constants::DynamicObjectType::Rabbit:
        type_name = "rabbit";
        break;
    case Constants::DynamicObjectType::People:
        type_name = "people";
        break;
    case Constants::DynamicObjectType::Wood:
        type_name = "wood";
        break;
    default:
        type_name = "car";
        break;
    }
    switch (color)
    {
    case Constants::ObjectColor::Red:
        color_name = "red";
        break;
    case Constants::ObjectColor::Blue:
        color_name = "blue";
        break;
    case Constants::ObjectColor::Green:
        color_name = "green";
        break;
    case Constants::ObjectColor::Grey:
        color_name = "grey";
        break;
    case Constants::ObjectColor::None:
        color_name = "none";
        break;
    default:
        color_name = "none";
        break;
    }
    std::string final_path = "assets/textures/" + type_name + "/" + color_name + "_" + frame + ".png";
    return getInstance().getTexture(final_path);
}

const sf::Texture &ResourceManager::getStaticObjectTexture(const Constants::StaticObjectType &type, const Constants::ObjectColor &color)
{
    std::string type_name, color_name;
    switch (type)
    {
    case Constants::StaticObjectType::Rock:
        type_name = "rock";
        break;
    case Constants::StaticObjectType::Bush:
        type_name = "bush";
        break;
    case Constants::StaticObjectType::Traffic:
        type_name = "traffic";
        break;
    case Constants::StaticObjectType::Coin:
        type_name = "coin";
        break;
    default:
        type_name = "rock";
        break;
    }
    switch (color)
    {
    case Constants::ObjectColor::Red:
        color_name = "red";
        break;
    case Constants::ObjectColor::Blue:
        color_name = "blue";
        break;
    case Constants::ObjectColor::Green:
        color_name = "green";
        break;
    case Constants::ObjectColor::Grey:
        color_name = "grey";
        break;
    case Constants::ObjectColor::None:
        color_name = "none";
        break;
    default:
        color_name = "none";
        break;
    }
    std::string final_path = "assets/textures/" + type_name + "/" + color_name + ".png";
    return getInstance().getTexture(final_path);
}

const sf::Texture &ResourceManager::getLaneTexture(const Constants::LaneType &type, int texture_variant)
{
    if (texture_variant < 1 || texture_variant > Constants::getNumberOfTextureVariant(type))
    {
        texture_variant = 1;
    }

    std::string type_name, texture_var;
    texture_var = std::to_string(texture_variant);
    switch (type)
    {
    case Constants::LaneType::Grass:
        type_name = "grass";
        break;
    case Constants::LaneType::Road:
        type_name = "road";
        break;
    case Constants::LaneType::Safe:
        type_name = "safe";
        break;
    case Constants::LaneType::River:
        type_name = "river";
        break;
    default:
        type_name = "grass";
        break;
    }
    std::string final_path = "assets/textures/lane/" + type_name + "_" + texture_var + ".png";
    return getInstance().getTexture(final_path);
}

const sf::Texture &ResourceManager::getTrafficTexture(const Constants::TrafficLightColor &color)
{
    std::string final_path = "assets/textures/traffic/traffic.png";
    return getInstance().getTexture(final_path);
}

const sf::IntRect ResourceManager::getTrafficTextureRect(const Constants::TrafficLightColor &color)
{
    const int FRAME_WIDTH = 800;
    const int FRAME_HEIGHT = 800;
    int column;
    switch (color)
    {
    case Constants::TrafficLightColor::Red:
        column = 0;
        break;
    case Constants::TrafficLightColor::Yellow:
        column = 1;
        break;
    case Constants::TrafficLightColor::Green:
        column = 2;
        break;
    default:
        column = 2;
        break;
    }
    return sf::IntRect(sf::Vector2i(column * FRAME_WIDTH, 0), sf::Vector2i(FRAME_WIDTH, FRAME_HEIGHT));
}

bool ResourceManager::hasSpritesheet(const Constants::DynamicObjectType &type) const
{
    return type == Constants::DynamicObjectType::Truck
        || type == Constants::DynamicObjectType::Car
        || type == Constants::DynamicObjectType::Rabbit
        || type == Constants::DynamicObjectType::Bird;
}

const sf::Texture &ResourceManager::getDynamicObjectSpritesheet(const Constants::DynamicObjectType &type, const Constants::ObjectColor &color)
{
    (void)color;
    std::string type_name;
    switch (type)
    {
    case Constants::DynamicObjectType::Truck:
        type_name = "truck";
        break;
    case Constants::DynamicObjectType::Car:
        type_name = "car";
        break;
    case Constants::DynamicObjectType::Rabbit:
        type_name = "rabbit";
        break;
    case Constants::DynamicObjectType::Bird:
        type_name = "bird";
        break;
    default:
        type_name = "truck";
        break;
    }
    std::string path = "assets/textures/" + type_name + "/none_1.png";
    return getTexture(path);
}

sf::IntRect ResourceManager::getDynamicObjectSpritesheetRect(const Constants::DynamicObjectType &type, int frame)
{
    const int FRAME_SIZE = 800;
    int numFrames = Constants::getNumberOfFrames(type);
    int idx = (frame / 4) % numFrames;
    const sf::Texture &tex = getDynamicObjectSpritesheet(type, Constants::ObjectColor::None);
    int cols = tex.getSize().x / FRAME_SIZE;
    int row = idx / cols;
    int col = idx % cols;

    int contentW, contentH;
    switch (type)
    {
    case Constants::DynamicObjectType::Car:
        contentW = 750; contentH = 425; break;
    case Constants::DynamicObjectType::Rabbit:
        contentW = 700; contentH = 700; break;
    case Constants::DynamicObjectType::Bird:
        contentW = 725; contentH = 643; break;
    case Constants::DynamicObjectType::Truck:
        contentW = 725; contentH = 675; break;
    default:
        contentW = FRAME_SIZE; contentH = FRAME_SIZE; break;
    }
    int offsetX = (FRAME_SIZE - contentW) / 2;
    int offsetY = (FRAME_SIZE - contentH) / 2;

    return sf::IntRect(sf::Vector2i(col * FRAME_SIZE + offsetX, row * FRAME_SIZE + offsetY),
                       sf::Vector2i(contentW, contentH));
}

void ResourceManager::clearAll()
{
    textures_.clear();
    fonts_.clear();
    sound_buffers_.clear();
}