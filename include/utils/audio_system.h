#pragma once

#include <SFML/Audio.hpp>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>

class AudioSystem
{
public:
    // Sử dụng Enum thay vì chuỗi (string) tĩnh để quản lý hiệu ứng âm thanh (SFX) và nhạc nền (Music)
    // Giúp dễ dàng gọi hàm và tránh lỗi đánh máy
    enum class SFX
    {
        Click,
        Hover,
        Back,
        Jump,
        Crash,
        GameOver,
        HighScore,
        Bird,
        Rabbit,
        CarHorn,
        TruckHorn
    };

    enum class Music
    {
        Background
    };

private:
    std::list<std::unique_ptr<sf::Sound>> active_sounds_;
    std::unique_ptr<sf::Music> bg_music_;
    bool is_music_loaded_ = false;

    float sfx_volume_ = 70.f;
    float music_volume_ = 50.f;

    // Map ánh xạ từ Enum sang đường dẫn file âm thanh tương ứng
    static const std::unordered_map<SFX, std::string> sfx_paths_;
    static const std::unordered_map<Music, std::string> music_paths_;

    AudioSystem();

public:
    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    static AudioSystem& getInstance()
    {
        static AudioSystem instance;
        return instance;
    }

    void setSFXVolume(int volume_level);
    void setMusicVolume(int volume_level);

    float getSFXVolume() const { return sfx_volume_; }
    float getMusicVolume() const { return music_volume_; }

    // Nạp sẵn toàn bộ file âm thanh (SFX) vào bộ nhớ RAM khi khởi động game
    // Việc này giúp tránh tình trạng giật lag mỗi lần phải load file mới lúc đang chơi
    void preloadAll();

    // Các hàm phát âm thanh, truyền vào Enum thay vì tên file
    void playSFX(SFX sfx);
    void playMusic(Music music);

    void pauseMusic();  // Tạm dừng phát nhạc nền
    void resumeMusic(); // Tiếp tục phát nhạc nền từ vị trí tạm dừng
    void stopAllSFX();  // Dừng mọi hiệu ứng âm thanh đang phát (vd: dùng khi game over)

    void update();
    void clearAll();
};

inline AudioSystem& Audio() { return AudioSystem::getInstance(); }