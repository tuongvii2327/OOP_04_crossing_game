#include "states/game_over_state.h"
#include "states/home_state.h"
#include "utils/audio_system.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"
#include <algorithm>
#include <fstream>
#include <iostream>

GameOverState::GameOverState(SharedContext* context) : State(context), selected_index_(0) {}

void GameOverState::init()
{
    Audio().pauseMusic();
    int top_score_before = 0;
    if (!context_->data->high_scores.empty())
    {
        top_score_before = context_->data->high_scores[0];
    }

    bool is_new_highscore =
        (top_score_before > 0) && (context_->data->current_score > top_score_before);
    if (is_new_highscore)
    {
        Audio().resumeMusic();
    }
    else
    {
        // Audio().pauseMusic();
        Audio().playSFX(AudioSystem::SFX::GameOver);
    }

    // save highscore when die
    saveHighScore();

    const sf::Font& font = Resource().getFont("assets/fonts/VT323-Regular.ttf");
    title_text_ = std::make_unique<sf::Text>(font);
    score_text_ = std::make_unique<sf::Text>(font);

    setupUI(font);

    if (is_new_highscore)
    {
        *title_text_ = Resource().createText("NEW HIGHSCORE!", {110, sf::Color::Magenta});
        sf::FloatRect title_bounds = title_text_->getLocalBounds();
        title_text_->setPosition({Constants::WINDOW_WIDTH / 2.f, 220.f + title_bounds.size.y / 2.f});
    }
}

void GameOverState::saveHighScore()
{
    int current_score = context_->data->current_score;
    auto& scores = context_->data->high_scores;

    scores.push_back(current_score);
    std::sort(scores.begin(), scores.end(), std::greater<int>());

    if (scores.size() > 5)
    {
        scores.resize(5);
    }

    std::ofstream file("data/highscore.bin", std::ios::binary);
    if (file.is_open())
    {
        file.write(reinterpret_cast<const char*>(scores.data()), 5 * sizeof(int));
        file.close();
    }
}

void GameOverState::setupUI(const sf::Font& font)
{
    background_overlay_.setSize({static_cast<float>(Constants::WINDOW_WIDTH),
                                 static_cast<float>(Constants::WINDOW_HEIGHT)});
    background_overlay_.setFillColor(sf::Color(0, 0, 0, 180)); // 180/255 độ tối

    *title_text_ = Resource().createText("GAME OVER", {110, sf::Color::Red});
    sf::FloatRect title_bounds = title_text_->getLocalBounds();
    title_text_->setPosition({Constants::WINDOW_WIDTH / 2.f, 220.f + title_bounds.size.y / 2.f});

    *score_text_ = Resource().createText(
        "YOUR SCORE: " + std::to_string(context_->data->current_score), {60, sf::Color::Yellow});
    sf::FloatRect score_bounds = score_text_->getLocalBounds();
    score_text_->setPosition({Constants::WINDOW_WIDTH / 2.f, 400.f + score_bounds.size.y / 2.f});

    std::vector<std::string> options = {"TRY AGAIN", "MAIN MENU"};
    float initial_y = 550.f;
    float spacing = 100.f;

    menu_texts_.clear();
    for (size_t i = 0; i < options.size(); ++i)
    {
        auto text = std::make_unique<sf::Text>(Resource().createText(options[i], {60}, true));

        if (static_cast<int>(i) == selected_index_)
            Resource().setTextButtonHovered(*text, true);

        sf::FloatRect bounds = text->getLocalBounds();
        text->setPosition(
            {Constants::WINDOW_WIDTH / 2.f, initial_y + i * spacing + bounds.size.y / 2.f});
        menu_texts_.push_back(std::move(text));
    }
}

void GameOverState::handleEvent(const sf::Event& event)
{
    if (const auto* key_pressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (key_pressed->code == sf::Keyboard::Key::Up || key_pressed->code == sf::Keyboard::Key::W)
        {
            moveUp();
        }
        else if (key_pressed->code == sf::Keyboard::Key::Down ||
                 key_pressed->code == sf::Keyboard::Key::S)
        {
            moveDown();
        }
        else if (key_pressed->code == sf::Keyboard::Key::Enter)
        {
            if (selected_index_ == 0)
            {
                Audio().playSFX(AudioSystem::SFX::Click);
                context_->state_machine->pop();
            }
            else
            {
                Audio().playSFX(AudioSystem::SFX::Click);
                context_->state_machine->resetTo(std::make_unique<HomeState>(context_));
            }
        }
    }

    if (const auto* mouse_moved = event.getIf<sf::Event::MouseMoved>())
    {
        sf::Vector2f mouse_pos = context_->window->mapPixelToCoords(mouse_moved->position);

        for (size_t i = 0; i < menu_texts_.size(); ++i)
        {
            if (menu_texts_[i]->getGlobalBounds().contains(mouse_pos))
            {
                if (selected_index_ != static_cast<int>(i))
                {
                    Resource().setTextButtonHovered(*menu_texts_[selected_index_], false);
                    selected_index_ = static_cast<int>(i);
                    Resource().setTextButtonHovered(*menu_texts_[selected_index_], true);
                }
                break;
            }
        }
    }

    if (const auto* mouse_button_pressed = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mouse_button_pressed->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mouse_pos = context_->window->mapPixelToCoords(mouse_button_pressed->position);

            for (size_t i = 0; i < menu_texts_.size(); ++i)
            {
                if (menu_texts_[i]->getGlobalBounds().contains(mouse_pos))
                {
                    selected_index_ = static_cast<int>(i);

                    if (selected_index_ == 0)
                    {
                        Audio().playSFX(AudioSystem::SFX::Click);
                        context_->state_machine->pop();
                    }
                    else
                    {
                        Audio().playSFX(AudioSystem::SFX::Click);
                        context_->state_machine->resetTo(std::make_unique<HomeState>(context_));
                    }
                    break;
                }
            }
        }
    }
}

void GameOverState::update(float delta_time) {}

void GameOverState::render(sf::RenderWindow& window)
{
    window.draw(background_overlay_);
    window.draw(*title_text_);
    window.draw(*score_text_);
    for (const auto& text : menu_texts_)
    {
        window.draw(*text);
    }
}

void GameOverState::moveUp()
{
    Resource().setTextButtonHovered(*menu_texts_[selected_index_], false);
    selected_index_--;
    if (selected_index_ < 0)
        selected_index_ = static_cast<int>(menu_texts_.size()) - 1;
    Resource().setTextButtonHovered(*menu_texts_[selected_index_], true);
}

void GameOverState::moveDown()
{
    Resource().setTextButtonHovered(*menu_texts_[selected_index_], false);
    selected_index_++;
    if (selected_index_ >= static_cast<int>(menu_texts_.size()))
        selected_index_ = 0;
    Resource().setTextButtonHovered(*menu_texts_[selected_index_], true);
}
