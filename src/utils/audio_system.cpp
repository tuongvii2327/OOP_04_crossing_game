#include "utils/audio_system.h"
#include "utils/resource_manager.h"
#include "utils/game_exceptions.h"
#include <iostream>

// Khởi tạo map liên kết giữa Enum và đường dẫn file âm thanh tương ứng
const std::unordered_map<AudioSystem::SFX, std::string> AudioSystem::sfx_paths_ = {
    {SFX::Click, "assets/audio/select.wav"},
    {SFX::Hover, "assets/audio/hover.wav"},
    {SFX::Back, "assets/audio/back.wav"},
    {SFX::Jump, "assets/audio/jump.wav"},
    {SFX::Crash, "assets/audio/crash.wav"},
    {SFX::GameOver, "assets/audio/game_over.wav"},
    {SFX::HighScore, "assets/audio/new_highscore.wav"},
    {SFX::Bird, "assets/audio/bird.wav"},
    // {SFX::Rabbit, "assets/audio/car_horn.wav"}, // TODO: Tạm thời không bật tiếng thỏ nhảy
    {SFX::CarHorn, "assets/audio/car_horn.wav"},
    {SFX::TruckHorn, "assets/audio/truck_horn.wav"}};

const std::unordered_map<AudioSystem::Music, std::string> AudioSystem::music_paths_ = {
    {Music::Background, "assets/audio/bg_music.ogg"}};

AudioSystem::AudioSystem() : bg_music_(std::make_unique<sf::Music>()) {}

void AudioSystem::preloadAll()
{
    for (const auto& pair : sfx_paths_)
    {
        try
        {
            Resource().getSoundBuffer(pair.second);
        }
        catch (const ResourceException& e)
        {
            std::cerr << "[AudioSystem] Preload lỗi: " << e.what() << '\n';
        }
    }
}

void AudioSystem::setSFXVolume(int volume_level)
{
    sfx_volume_ = static_cast<float>(volume_level) * 10.f;
    for (auto& sound : active_sounds_)
    {
        if (sound)
            sound->setVolume(sfx_volume_);
    }
}

void AudioSystem::setMusicVolume(int volume_level)
{
    music_volume_ = static_cast<float>(volume_level) * 10.f;
    if (is_music_loaded_ && bg_music_)
    {
        bg_music_->setVolume(music_volume_);
    }
}

void AudioSystem::playSFX(SFX sfx)
{
    // return;
    try
    {
        auto it = sfx_paths_.find(sfx);
        if (it == sfx_paths_.end())
            return;

        const sf::SoundBuffer& buffer = Resource().getSoundBuffer(it->second);

        // Cấp phát đối tượng âm thanh độc lập trên Heap để đảm bảo nó luôn sống cho tới khi phát xong
        auto sound = std::make_unique<sf::Sound>(buffer);
        sound->setVolume(sfx_volume_);
        sound->play();
        
        // Đẩy vào danh sách các âm thanh đang phát để quản lý
        active_sounds_.push_back(std::move(sound));
    }
    catch (const ResourceException& e)
    {
        std::cerr << "[AudioSystem] Lỗi phát SFX: " << e.what() << '\n';
    }
}

void AudioSystem::playMusic(Music music)
{
    auto it = music_paths_.find(music);
    if (it == music_paths_.end())
        return;

    if (!bg_music_->openFromFile(it->second))
    {
        std::cerr << "[AudioSystem] Lỗi tải nhạc nền: " << it->second << '\n';
        is_music_loaded_ = false;
        return;
    }
    is_music_loaded_ = true;
    bg_music_->setVolume(music_volume_);
    bg_music_->setLooping(true);
    bg_music_->play();
}

void AudioSystem::pauseMusic()
{
    if (is_music_loaded_ && bg_music_ && bg_music_->getStatus() == sf::Music::Status::Playing)
    {
        bg_music_->pause();
    }
}

void AudioSystem::resumeMusic()
{
    if (!is_music_loaded_ || !bg_music_)
        return;

    if (bg_music_->getStatus() != sf::Music::Status::Playing)
    {
        bg_music_->play();
    }
}

void AudioSystem::stopAllSFX()
{
    // Lệnh clear() sẽ hủy toàn bộ các con trỏ std::unique_ptr<sf::Sound> đang tồn tại,
    // khiến tất cả các âm thanh SFX đang phát bị tắt ngay lập tức.
    active_sounds_.clear();
}

void AudioSystem::update()
{
    // Dọn dẹp các con trỏ âm thanh đã phát xong hoặc bị lỗi để giải phóng bộ nhớ
    active_sounds_.remove_if(
        [](const std::unique_ptr<sf::Sound>& sound)
        { return !sound || sound->getStatus() == sf::Sound::Status::Stopped; });
}

void AudioSystem::clearAll()
{
    active_sounds_.clear();
    if (is_music_loaded_ && bg_music_)
    {
        bg_music_->stop();
    }
}