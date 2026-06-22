#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>


class ResourceManager
{
private:
    // Asset caches utilizing unique_ptr to ensure automatic memory deallocation on shutdown
    std::unordered_map<std::string, std::unique_ptr<sf::Texture>> textures_;
    std::unordered_map<std::string, std::unique_ptr<sf::Font>> fonts_;
    std::unordered_map<std::string, std::unique_ptr<sf::SoundBuffer>> sound_buffers_;

    // Private constructor prevents external instantiation enforcing the Singleton pattern
    ResourceManager() = default;

public:
    // Delete copy constructor and copy assignment operator to avoid duplicate instances
    ResourceManager(const ResourceManager &) = delete;
    ResourceManager &operator=(const ResourceManager &) = delete;

    // Retrieve the global single instance of the resource manager
    static ResourceManager &getInstance();

    // ─── ASSET ACQUISITION METHODS (Returns a 'const &' reference) ───
    const sf::Texture &getTexture(const std::string &filepath);
    const sf::Font &getFont(const std::string &filepath);
    const sf::SoundBuffer &getSoundBuffer(const std::string &filepath);

    // ─── PRE-LOADING UTILITIES (Ideal for loading screens to cache assets early) ───
    void preloadTexture(const std::string &filepath) { (void)getTexture(filepath); }
    void preloadFont(const std::string &filepath) { (void)getFont(filepath); }
    void preloadSoundBuffer(const std::string &filepath) { (void)getSoundBuffer(filepath); }

    // Wipe all internal caches to free up system memory and VRAM
    void clearAll();
};

// Global helper function shortcut. Instead of ResourceManager::getInstance().getTexture(...)
// You can cleanly write: Resource().getTexture(...)
inline ResourceManager &Resource()
{
    return ResourceManager::getInstance();
}