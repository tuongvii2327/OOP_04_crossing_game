#pragma once

#include <SFML/Audio.hpp>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>

class AudioSystem
{
public:
    // Định nghĩa Enum cho SFX và Music thay vì dùng chuỗi tĩnh
    enum class SFX
    {
        Click,
        Hover,
        Back,
        Jump,
        Crash,
        Bird,
        Rabbit,
        Horn
    };

    enum class Music
    {
        Background
    };

private:
    std::list<std::unique_ptr<sf::Sound>> active_sounds_;
    std::unique_ptr<sf::Music> bg_music_;

    float sfx_volume_ = 70.f;
    float music_volume_ = 50.f;

    // Map chứa đường dẫn cố định của file âm thanh
    static const std::unordered_map<SFX, std::string> sfx_paths_;
    static const std::unordered_map<Music, std::string> music_paths_;

    AudioSystem();

public:
    AudioSystem(const AudioSystem &) = delete;
    AudioSystem &operator=(const AudioSystem &) = delete;

    static AudioSystem &getInstance()
    {
        static AudioSystem instance;
        return instance;
    }

    void setSFXVolume(int volume_level);
    void setMusicVolume(int volume_level);

    // Tải trước tất cả SFX vào RAM để chống giật lag khi chơi
    void preloadAll();

    // Phát âm thanh sử dụng Enum thay vì String
    void playSFX(SFX sfx);
    void playMusic(Music music);

    void update();
    void clearAll();
};

inline AudioSystem &Audio() { return AudioSystem::getInstance(); }