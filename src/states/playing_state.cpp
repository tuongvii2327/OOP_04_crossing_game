#include "states/playing_state.h"
#include "states/pause_state.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"
#include <iostream>

PlayingState::PlayingState(SharedContext *context)
    : State(context), score_(0), is_resuming_(false) {}

void PlayingState::init()
{
    std::cout << "[PlayingState] Initializing...\n";

    // 1. Manage Font via Resource Manager
    const sf::Font &font = Resource().getFont("assets/fonts/VT323-Regular.ttf");

    score_text_ = std::make_unique<sf::Text>(font);
    score_text_->setString("Score: 0");
    score_text_->setCharacterSize(40);
    score_text_->setFillColor(sf::Color::White);
    score_text_->setOutlineColor(sf::Color::Black);
    score_text_->setOutlineThickness(2.f);
    score_text_->setPosition({20.f, 20.f});

    // 2. Initialize the player entity by passing the texture from the resource cache
    const sf::Texture &player_tex = Resource().getTexture("assets/textures/player.png");
    player_ = std::make_unique<People>(player_tex);

    // 3. Initialize the map background, expanding upwards starting from the screen bottom
    map_.initMap(Constants::WINDOW_HEIGHT - Constants::LANE_HEIGHT);

    // 4. Configure the initial default camera view dimensions and center position
    view_.setSize({static_cast<float>(Constants::WINDOW_WIDTH), static_cast<float>(Constants::WINDOW_HEIGHT)});
    view_.setCenter({static_cast<float>(Constants::WINDOW_WIDTH) / 2.f, static_cast<float>(Constants::WINDOW_HEIGHT) / 2.f});
}

void PlayingState::handleEvent(const sf::Event &event)
{
    if (const auto *key_pressed = event.getIf<sf::Event::KeyPressed>())
    {
        // Press ESC or P to open the screen-overlay Pause Menu
        if (key_pressed->code == sf::Keyboard::Key::P || key_pressed->code == sf::Keyboard::Key::Escape)
        {
            context_->state_machine->addState(std::make_unique<PauseState>(context_), false); // Đổi ở đây
            return;
        }

        // Ensure player_ is safely initialized before processing movement
        if (!player_)
            return;

        float step = static_cast<float>(Constants::LANE_HEIGHT);

        float cam_top = view_.getCenter().y - (view_.getSize().y / 2.f);
        if (key_pressed->code == sf::Keyboard::Key::W || key_pressed->code == sf::Keyboard::Key::Up)
        {
            if (player_->tryToMove(0.f, -step, cam_top))
            {
                player_->addScore();
                score_ = player_->getScore();
                score_text_->setString("Score: " + std::to_string(score_));
            }
        }
        else if (key_pressed->code == sf::Keyboard::Key::S || key_pressed->code == sf::Keyboard::Key::Down)
        {
            player_->tryToMove(0.f, step, cam_top);
        }
        else if (key_pressed->code == sf::Keyboard::Key::A || key_pressed->code == sf::Keyboard::Key::Left)
        {
            player_->tryToMove(-step, 0.f, cam_top);
        }
        else if (key_pressed->code == sf::Keyboard::Key::D || key_pressed->code == sf::Keyboard::Key::Right)
        {
            player_->tryToMove(step, 0.f, cam_top);
        }
    }
}

void PlayingState::update(float dt)
{
    // If the game was just unpaused, skip this frame's update
    // to prevent massive Delta Time spikes from breaking entity positions
    if (is_resuming_)
    {
        is_resuming_ = false;
        return;
    }

    if (!player_)
        return;

    if (player_->isDead())
    {
        // stopping the game when player is dead, can be changed to transition to a GameOverState later
        return;
    }

    // Update player logic and animations
    player_->updateState(dt);

    // Camera calculation: Push the view center upwards when the player passes the halfway mark
    float current_center_y = view_.getCenter().y;
    if (player_->getPosition().y < current_center_y)
    {
        view_.setCenter({view_.getCenter().x, player_->getPosition().y});
    }

    // Extract the actual top and bottom boundaries of the current camera view
    float camera_top_y = view_.getCenter().y - (view_.getSize().y / 2.f);
    float camera_bottom_y = view_.getCenter().y + (view_.getSize().y / 2.f);

    // Update Map: Spawn lanes at the top, clear hidden lanes at the bottom, and advance vehicles
    map_.update(camera_top_y, camera_bottom_y, dt);

    if (!player_->isDead())
    {
        // crash cars
        if (map_.checkLethalCollision(player_->getHitBox()))
        {
            std::cout << "[PlayingState] PLAYER KILLED BY VEHICLE!\n";
            player_->die();

            context_->data->current_score = score_;

            context_->state_machine->addState(std::make_unique<GameOverState>(context_), false);
            return;
        }
        else if (player_->getPosition().y > camera_bottom_y)
        {
            std::cout << "[PlayingState] PLAYER SWEEPT BY CAMERA!\n";
            player_->die();

            context_->data->current_score = score_;
            context_->state_machine->addState(std::make_unique<GameOverState>(context_), false);
            return;
        }
    }
    else
    {
        // add later: trans to game over state
        std::cout << "Game Over! \n";
    }
}

void PlayingState::render(sf::RenderWindow &window)
{
    // STEP 1: Apply the dynamic mobile camera view to render the game world
    window.setView(view_);
    map_.render(window);

    if (player_)
    {
        player_->render(window);
    }

    // STEP 2: Restore the window to its default fixed view to draw the static HUD layer
    window.setView(window.getDefaultView());
    if (score_text_)
    {
        window.draw(*score_text_);
    }
}

void PlayingState::pause()
{
    std::cout << "[PlayingState] Paused\n";
}

void PlayingState::resume()
{
    std::cout << "[PlayingState] Resumed\n";
    // Trigger the resume flag to ignore the next heavily delayed frame update
    is_resuming_ = true;
}

// /// Add later in gameover_state
// #include <algorithm>
// #include <fstream>

// void updateLeaderboard(SharedContext *context, int new_score)
// {
//     auto &scores = context->data->high_scores;

//     scores.push_back(new_score);
//     std::sort(scores.begin(), scores.end(), std::greater<int>());

//     if (scores.size() > 5)
//     {
//         scores.resize(5);
//     }

//     std::ofstream file("data/highscore.bin", std::ios::binary);
//     if (file.is_open())
//     {
//         file.write(reinterpret_cast<const char *>(scores.data()), scores.size() * sizeof(int));
//         file.close();
//     }
// }