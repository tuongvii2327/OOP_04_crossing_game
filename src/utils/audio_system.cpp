#include "utils/audio_system.h"
#include "utils/resource_manager.h"
#include <iostream>

// Khởi tạo đường dẫn cho các âm thanh
const std::unordered_map<AudioSystem::SFX, std::string> AudioSystem::sfx_paths_ = {
    {SFX::Click, "assets/audio/select.wav"}, // https://pixabay.com/sound-effects/film-special-effects-90s-game-ui-6-185099/
    {SFX::Hover, "assets/audio/hover.wav"},  // https://pixabay.com/sound-effects/film-special-effects-ui-sounds-pack-3-15-359713/
    {SFX::Back, "assets/audio/back.wav"},    // https://pixabay.com/sound-effects/film-special-effects-ui-sound-374228/
    {SFX::Jump, "assets/audio/jump.wav"},    // https://pixabay.com/sound-effects/film-special-effects-pixel-sound-effect-4-82881/
    {SFX::Crash, "assets/audio/crash.wav"},  // https://pixabay.com/sound-effects/film-special-effects-sound-effect-car-crash-394903/
    {SFX::Bird, "assets/audio/horn.wav"},    // tam
    {SFX::Rabbit, "assets/audio/horn.wav"},  // tam
    {SFX::Horn, "assets/audio/horn.wav"}};   // https://pixabay.com/sound-effects/city-car-horn-6408/

const std::unordered_map<AudioSystem::Music, std::string> AudioSystem::music_paths_ = {
    {Music::Background, "assets/audio/bg_music.ogg"}}; // https://pixabay.com/music/upbeat-game-8-bit-399898/

AudioSystem::AudioSystem() : bg_music_(std::make_unique<sf::Music>()) {}

void AudioSystem::preloadAll()
{
    for (const auto &pair : sfx_paths_)
    {
        try
        {
            Resource().getSoundBuffer(pair.second);
        }
        catch (const std::exception &e)
        {
            std::cerr << "[AudioSystem] Preload lỗi: " << e.what() << '\n';
        }
    }
}

void AudioSystem::setSFXVolume(int volume_level)
{
    sfx_volume_ = static_cast<float>(volume_level) * 10.f;
    for (auto &sound : active_sounds_)
    {
        if (sound)
            sound->setVolume(sfx_volume_);
    }
}

void AudioSystem::setMusicVolume(int volume_level)
{
    music_volume_ = static_cast<float>(volume_level) * 10.f;
    bg_music_->setVolume(music_volume_);
}

void AudioSystem::playSFX(SFX sfx)
{
    // return;
    try
    {
        auto it = sfx_paths_.find(sfx);
        if (it == sfx_paths_.end())
            return;

        const sf::SoundBuffer &buffer = Resource().getSoundBuffer(it->second);

        // Cấp phát âm thanh độc lập trên Heap để đảm bảo nó luôn sống cho tới khi phát xong
        auto sound = std::make_unique<sf::Sound>(buffer);
        sound->setVolume(sfx_volume_);
        sound->play();
        active_sounds_.push_back(std::move(sound));
    }
    catch (const std::exception &e)
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
        return;
    }
    bg_music_->setVolume(music_volume_);
    bg_music_->setLooping(true);
    bg_music_->play();
}

void AudioSystem::update()
{
    active_sounds_.remove_if([](const std::unique_ptr<sf::Sound> &sound)
                             { return !sound || sound->getStatus() == sf::Sound::Status::Stopped; });
}

void AudioSystem::clearAll()
{
    active_sounds_.clear();
    bg_music_->stop();
}