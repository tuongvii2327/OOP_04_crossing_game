#include "utils/resource_manager.h"
#include <iostream>

ResourceManager &ResourceManager::getInstance()
{
    static ResourceManager instance; // Khởi tạo đúng 1 lần duy nhất, chuẩn C++11 thread-safe
    return instance;
}

const sf::Texture &ResourceManager::getTexture(const std::string &filepath)
{
    // 1. Tìm xem trong RAM đã load file này bao giờ chưa?
    auto it = textures_.find(filepath);
    if (it != textures_.end())
    {
        return *(it->second); // Đã có -> Trả về dùng luôn, tốc độ O(1)
    }

    // 2. Chưa có -> Bắt đầu đọc từ ổ cứng lên
    auto texture = std::make_unique<sf::Texture>();
    if (!texture->loadFromFile(filepath))
    {
        throw std::runtime_error("[ResourceManager] FATAL: Cannot load Texture -> " + filepath);
    }

    // 3. Cất vào kho và trả về tham chiếu
    auto inserted = textures_.emplace(filepath, std::move(texture));
    return *(inserted.first->second);
}

const sf::Font &ResourceManager::getFont(const std::string &filepath)
{
    auto it = fonts_.find(filepath);
    if (it != fonts_.end())
    {
        return *(it->second);
    }

    auto font = std::make_unique<sf::Font>();
    if (!font->openFromFile(filepath)) // Cú pháp chuẩn SFML 3
    {
        throw std::runtime_error("[ResourceManager] FATAL: Cannot load Font -> " + filepath);
    }

    auto inserted = fonts_.emplace(filepath, std::move(font));
    return *(inserted.first->second);
}

const sf::SoundBuffer &ResourceManager::getSoundBuffer(const std::string &filepath)
{
    auto it = sound_buffers_.find(filepath);
    if (it != sound_buffers_.end())
    {
        return *(it->second);
    }

    auto buffer = std::make_unique<sf::SoundBuffer>();
    if (!buffer->loadFromFile(filepath))
    {
        throw std::runtime_error("[ResourceManager] FATAL: Cannot load SoundBuffer -> " + filepath);
    }

    auto inserted = sound_buffers_.emplace(filepath, std::move(buffer));
    return *(inserted.first->second);
}

void ResourceManager::clearAll()
{
    textures_.clear();
    fonts_.clear();
    sound_buffers_.clear();
    std::cout << "[ResourceManager] VRAM/RAM cleared successfully.\n";
}