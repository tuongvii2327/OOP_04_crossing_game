#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>

class ResourceManager
{
private:
    // Các kho chứa tài nguyên, dùng unique_ptr để tự động hủy vùng nhớ khi tắt game
    std::unordered_map<std::string, std::unique_ptr<sf::Texture>> textures_;
    std::unordered_map<std::string, std::unique_ptr<sf::Font>> fonts_;
    std::unordered_map<std::string, std::unique_ptr<sf::SoundBuffer>> sound_buffers_;

    // Đưa Constructor vào private để cấm khởi tạo bừa bãi ngoài hàm getInstance()
    ResourceManager() = default;

public:
    // Xóa Copy Constructor và Copy Assignment (Luật bất thành văn của Singleton)
    ResourceManager(const ResourceManager &) = delete;
    ResourceManager &operator=(const ResourceManager &) = delete;

    // Lấy thực thể duy nhất của toàn bộ Game
    static ResourceManager &getInstance();

    // ─── CÁC HÀM LẤY TÀI NGUYÊN (Trả về tham chiếu hằng 'const &') ───
    const sf::Texture &getTexture(const std::string &filepath);
    const sf::Font &getFont(const std::string &filepath);
    const sf::SoundBuffer &getSoundBuffer(const std::string &filepath);

    // ─── HÀM HỖ TRỢ PRE-LOAD (Dùng cho màn hình LoadingScreen) ───
    void preloadTexture(const std::string &filepath) { (void)getTexture(filepath); }
    void preloadFont(const std::string &filepath) { (void)getFont(filepath); }
    void preloadSoundBuffer(const std::string &filepath) { (void)getSoundBuffer(filepath); }

    // Dọn dẹp toàn bộ RAM (Dùng khi muốn Reset toàn bộ game)
    void clearAll();
};

// Hàm Helper viết tắt cho gọn code. Thay vì viết: ResourceManager::getInstance().getTexture(...)
// Giờ chỉ cần viết: Resource().getTexture(...)
inline ResourceManager &Resource()
{
    return ResourceManager::getInstance();
}