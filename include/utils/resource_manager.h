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
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    static ResourceManager& getInstance(); // Sử dụng Singleton pattern để truy cập

    // Các hàm tải (hoặc trả về nếu đã cache) tài nguyên từ đường dẫn file tương ứng
    const sf::Texture& getTexture(const std::string& filepath); 
    const sf::Font& getFont(const std::string& filepath);
    const sf::SoundBuffer& getSoundBuffer(const std::string& filepath);

    // Lấy texture cho các đối tượng động (dynamic objects) dựa trên loại, màu sắc và frame hiện tại
    const sf::Texture& getDynamicObjectTexture(
        const Constants::DynamicObjectType& type,
        const Constants::ObjectColor& color = Constants::ObjectColor::None,
        int cur_frame = 1); 

    // Lấy texture cho các đối tượng tĩnh (static objects) dựa trên loại và màu sắc
    const sf::Texture& getStaticObjectTexture(
        const Constants::StaticObjectType& type,
        const Constants::ObjectColor& color = Constants::ObjectColor::None); 

    // Lấy texture cho các loại làn đường (lane)
    const sf::Texture& getLaneTexture(
        const Constants::LaneType& type,
        int texture_variant); 

    // Lấy texture gốc của đèn giao thông (spritesheet)
    const sf::Texture& getTrafficTexture(
        const Constants::TrafficLightColor& color); 

    // Lấy khung hinh (IntRect) ứng với màu đèn giao thông trên spritesheet
    const sf::IntRect getTrafficTextureRect(
        const Constants::TrafficLightColor& color);

    bool hasSpritesheet(const Constants::DynamicObjectType& type) const;
    const sf::Texture& getDynamicObjectSpritesheet(const Constants::DynamicObjectType& type,
                                                   const Constants::ObjectColor& color);
    sf::IntRect getDynamicObjectSpritesheetRect(const Constants::DynamicObjectType& type,
                                                int frame);

    // Xóa toàn bộ tài nguyên đã được cache.
    // Lưu ý: Tất cả các sf::Sprite hay sf::Text đang dùng tài nguyên này sẽ bị lỗi (dangling),
    // do đó chỉ nên gọi hàm này khi thoát chương trình.
    void clearAll();

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

inline ResourceManager& Resource() { return ResourceManager::getInstance(); }
