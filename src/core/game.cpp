#include "core/game.h"
#include "states/home_state.h"
#include "utils/audio_system.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"
#include "utils/serialization.h"
#include <algorithm>
#include <fstream>
#include <iostream>

Game::Game()
{
    window.create(sf::VideoMode({Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT}),
                  Constants::GAME_TITLE,
                  sf::Style::Titlebar | sf::Style::Close);

    // set FPS for game
    window.setFramerateLimit(Constants::FPS_LIMIT);

    // Thêm hàm preload
    Audio().preloadAll();
    Audio().setMusicVolume(config_.music_volume);
    Audio().setSFXVolume(config_.sfx_volume);
    // Thay thế gọi String bằng Enum
    Audio().playMusic(AudioSystem::Music::Background);

    context_.window = &window;
    context_.state_machine = &state_machine;
    context_.config = &config_;
    context_.data = &data_;

    std::ifstream file("data/highscore.bin", std::ios::binary);
    if (file.is_open())
    {
        data_.high_scores.resize(5);
        file.read(reinterpret_cast<char *>(data_.high_scores.data()), 5 * sizeof(int));
        file.close();
    }

    while (data_.high_scores.size() < 5)
    {
        data_.high_scores.push_back(0);
    }

    std::sort(data_.high_scores.begin(), data_.high_scores.end(), std::greater<int>());

    loadPlayerData();
    state_machine.push(std::make_unique<HomeState>(&context_));
}

Game::~Game()
{
    savePlayerData();

    state_machine.resetTo(nullptr);
    state_machine.processTransitions();

    Audio().clearAll();
    Resource().clearAll();
}

void Game::run()
{
    // std::cout << "Game is running..." << std::endl;
    while (window.isOpen())
    {
        // std::cout << "Looping..." << std::endl;
        if (state_machine.processTransitions())
        {
            // Reset clock when a state change occurs to avoid large delta time on the first update of the new state
            clock.restart();
        }

        // If there are no states left, close the window and exit the game loop
        if (state_machine.isEmpty())
        {
            window.close();
            break;
        }

        float dt = clock.restart().asSeconds();
        handleEvent();
        update(dt);
        render();
    }
}

void Game::handleEvent()
{
    while (const std::optional<sf::Event> event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            window.close();
            return;
        }

        if (!state_machine.isEmpty())
        {
            state_machine.getActiveState().handleEvent(*event);
        }
    }
}

void Game::update(float dt)
{
    Audio().update();
    state_machine.getActiveState().update(dt);
}

void Game::render()
{
    window.clear(sf::Color::Black);
    state_machine.render(window);
    window.display();
}

void Game::loadPlayerData()
{
    std::ifstream file("data/player_data.bin", std::ios::binary);
    if (file.is_open())
    {
        Serialization::read(file, data_.total_coins);
        Serialization::read(file, data_.selected_character);

        // Đọc trạng thái mở khóa của 4 nhân vật
        for (int i = 0; i < 4; ++i)
        {
            bool is_unlocked;
            Serialization::read(file, is_unlocked);
            data_.unlocked_characters[i] = is_unlocked;
        }

        file.close();
        std::cout << "[Game] Loaded Total Coins: " << data_.total_coins << "\n";
    }
    else
    {
        data_.total_coins = 0; // Mặc định là 0 nếu chưa có file dữ liệu
        data_.selected_character = 0;
        data_.unlocked_characters = {true, false, false, false};
    }
}

void Game::savePlayerData()
{
    std::ofstream file("data/player_data.bin", std::ios::binary);
    if (file.is_open())
    {
        Serialization::write(file, data_.total_coins);
        Serialization::write(file, data_.selected_character);

        // Ghi trạng thái mở khóa của 4 nhân vật
        for (int i = 0; i < 4; ++i)
        {
            bool is_unlocked = data_.unlocked_characters[i];
            Serialization::write(file, is_unlocked);
        }

        file.close();
        std::cout << "[Game] Saved Total Coins: " << data_.total_coins << "\n";
    }
}