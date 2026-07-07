#include "states/playing_state.h"
#include "states/game_over_state.h"
#include "states/pause_state.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"
#include <iostream>

PlayingState::PlayingState(SharedContext *context)
    : State(context)
{
}

void PlayingState::init()
{
    std::cout << "[PlayingState] Initializing...\n";

    const sf::Font &font = Resource().getFont("assets/fonts/VT323-Regular.ttf");

    score_text_ = std::make_unique<sf::Text>(font);
    score_text_->setString("Score: 0");
    score_text_->setCharacterSize(40);
    score_text_->setFillColor(sf::Color::White);
    score_text_->setOutlineColor(sf::Color::Black);
    score_text_->setOutlineThickness(2.f);
    score_text_->setPosition({20.f, 20.f});

    const sf::Texture &player_tex =
        Resource().getTexture("assets/textures/player.png");
    player_ = std::make_unique<People>(player_tex);

    setupWorld();
}

void PlayingState::setupWorld()
{
    map_.reset(Constants::WINDOW_HEIGHT - Constants::LANE_HEIGHT);

    view_.setSize({static_cast<float>(Constants::WINDOW_WIDTH),
                   static_cast<float>(Constants::WINDOW_HEIGHT)});
    view_.setCenter({static_cast<float>(Constants::WINDOW_WIDTH) / 2.f,
                     static_cast<float>(Constants::WINDOW_HEIGHT) / 2.f});
}

void PlayingState::handleEvent(const sf::Event &event)
{
    if (const auto *key_pressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (key_pressed->code == sf::Keyboard::Key::P ||
            key_pressed->code == sf::Keyboard::Key::Escape)
        {
            context_->state_machine->push(std::make_unique<PauseState>(context_));
            return;
        }
    }

    if (!player_)
        return;

    float cam_top = view_.getCenter().y - (view_.getSize().y / 2.f);
    if (player_->handleEvent(event, cam_top))
    {
        score_text_->setString("Score: " +
                               std::to_string(context_->data->current_score));
    }
}

void PlayingState::update(float dt)
{
    if (!player_ || player_->isDead())
        return;

    player_->updateState(dt);

    float start_y = Constants::WINDOW_HEIGHT - (Constants::LANE_HEIGHT * 1.5f) -
                    (player_->getHeightSprite() / 2.f);
    float current_y = player_->getPosition().y;

    int distance_score =
        static_cast<int>((start_y - current_y) / Constants::LANE_HEIGHT);

    if (distance_score > context_->data->current_score)
    {
        context_->data->current_score = distance_score;
        score_text_->setString("Score: " +
                               std::to_string(context_->data->current_score));
    }

    float current_center_y = view_.getCenter().y;
    if (player_->getPosition().y < current_center_y)
    {
        view_.setCenter({view_.getCenter().x, player_->getPosition().y});
    }

    float camera_top_y = view_.getCenter().y - (view_.getSize().y / 2.f);
    float camera_bottom_y = view_.getCenter().y + (view_.getSize().y / 2.f);

    map_.update(camera_top_y, camera_bottom_y, dt);

    if (map_.checkLethalCollision(player_->getHitBox()) ||
        (player_->getPosition().y + player_->getHeightSprite() * 0.5f) >= camera_bottom_y)
    {
        player_->die();
        context_->state_machine->push(std::make_unique<GameOverState>(context_));
        return;
    }
}

void PlayingState::render(sf::RenderWindow &window)
{
    window.setView(view_);
    map_.render(window);

    if (player_)
    {
        player_->render(window);
    }

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
    if (player_ && player_->isDead())
    {
        restart();
    }
}

void PlayingState::restart()
{
    std::cout << "[PlayingState] Restarting Game...\n";

    context_->data->current_score = 0;
    score_text_->setString("Score: 0");

    player_->resetPosition();

    setupWorld();
}
