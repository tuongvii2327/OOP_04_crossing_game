#include "utils/resource_manager.h"
#include <iostream>

ResourceManager &ResourceManager::getInstance()
{
    // Thread-safe initialization guaranteed by C++11 (Meyers Singleton)
    static ResourceManager instance;
    return instance;
}

const sf::Texture &ResourceManager::getTexture(const std::string &filepath)
{
    // 1. Check if the asset has already been loaded into memory
    auto it = textures_.find(filepath);
    if (it != textures_.end())
    {
        return *(it->second); // Cache hit -> Return immediately with O(1) complexity
    }

    // 2. Cache miss -> Load the asset from the storage drive
    auto texture = std::make_unique<sf::Texture>();
    if (!texture->loadFromFile(filepath))
    {
        throw std::runtime_error("[ResourceManager] FATAL: Cannot load Texture -> " + filepath);
    }

    // 3. Store the asset in the cache map and return its reference
    auto inserted = textures_.emplace(filepath, std::move(texture));
    return *(inserted.first->second);
}

const sf::Font &ResourceManager::getFont(const std::string &filepath)
{
    // 1. Check if the font has already been loaded
    auto it = fonts_.find(filepath);
    if (it != fonts_.end())
    {
        return *(it->second);
    }

    // 2. Load the font from disk using SFML 3 standard syntax
    auto font = std::make_unique<sf::Font>();
    if (!font->openFromFile(filepath))
    {
        throw std::runtime_error("[ResourceManager] FATAL: Cannot load Font -> " + filepath);
    }

    // 3. Store in cache and return reference
    auto inserted = fonts_.emplace(filepath, std::move(font));
    return *(inserted.first->second);
}

const sf::SoundBuffer &ResourceManager::getSoundBuffer(const std::string &filepath)
{
    // 1. Check if the sound buffer has already been loaded
    auto it = sound_buffers_.find(filepath);
    if (it != sound_buffers_.end())
    {
        return *(it->second);
    }

    // 2. Load the audio file from disk
    auto buffer = std::make_unique<sf::SoundBuffer>();
    if (!buffer->loadFromFile(filepath))
    {
        throw std::runtime_error("[ResourceManager] FATAL: Cannot load SoundBuffer -> " + filepath);
    }

    // 3. Store in cache and return reference
    auto inserted = sound_buffers_.emplace(filepath, std::move(buffer));
    return *(inserted.first->second);
}

void ResourceManager::clearAll()
{
    // Wipe all internal caches to free up system memory and VRAM
    textures_.clear();
    fonts_.clear();
    sound_buffers_.clear();
    std::cout << "[ResourceManager] VRAM/RAM cleared successfully.\n";
}