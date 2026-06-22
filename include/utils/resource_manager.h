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
private:
    // Bộ nhớ đệm cho file ổ cứng
    std::unordered_map<std::string, std::unique_ptr<sf::Texture>> textures_;
    std::unordered_map<std::string, std::unique_ptr<sf::Font>> fonts_;
    std::unordered_map<std::string, std::unique_ptr<sf::SoundBuffer>> sound_buffers_;

    ResourceManager() = default;

    // Các hàm Helper tự vẽ đồ họa (Procedural)
    static sf::Color toSFColor(const Constants::ObjectColor &color);
    static sf::Color toTrafficColor(const Constants::TrafficLightColor &color);
    static sf::Texture makeRoadTexture(sf::Color asphalt, int direction);
    static sf::Texture makeGrassTexture(sf::Color base);
    static sf::Texture makeVehicleTexture(unsigned w, unsigned h, sf::Color body, bool isTruck);
    static sf::Texture makeAnimalTexture(unsigned w, unsigned h, sf::Color body, bool isBird);
    static sf::Texture makeTrafficTexture(sf::Color lamp);
    static sf::Texture buildObjectTexture(const Constants::ObjectType &type, const Constants::ObjectColor &color);
    static sf::Texture buildLaneTexture(const Constants::LaneType &type, int direction);

public:
    ResourceManager(const ResourceManager &) = delete;
    ResourceManager &operator=(const ResourceManager &) = delete;

    static ResourceManager &getInstance();

    // ─── NHÓM 1: NẠP TỪ FILE (Dùng cho UI, Player, Font) ───
    const sf::Texture &getTexture(const std::string &filepath);
    const sf::Font &getFont(const std::string &filepath);
    const sf::SoundBuffer &getSoundBuffer(const std::string &filepath);

    // ─── NHÓM 2: TỰ VẼ ĐỘNG (Dùng cho Object, Xe, Thú, Lane) ───
    static const sf::Texture &getObjectTexture(const Constants::ObjectType &type, const Constants::ObjectColor &color);
    static const sf::Texture &getLaneTexture(const Constants::LaneType &type, int direction);
    static const sf::Texture &getTrafficTexture(const Constants::TrafficLightColor &color);

    void clearAll();
};

inline ResourceManager &Resource() { return ResourceManager::getInstance(); }