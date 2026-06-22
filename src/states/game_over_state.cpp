#include "states/game_over_state.h"
#include "states/home_state.h"
#include "states/playing_state.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"
#include <algorithm>
#include <fstream>
#include <iostream>

GameOverState::GameOverState(SharedContext *context)
    : State(context), selected_index_(0) {}

void GameOverState::init()
{
    // save highscore when die
    saveHighScore();

    const sf::Font &font = Resource().getFont("assets/fonts/VT323-Regular.ttf");
    title_text_ = std::make_unique<sf::Text>(font);
    score_text_ = std::make_unique<sf::Text>(font);

    setupUI(font);
}

void GameOverState::saveHighScore()
{
    int current_score = context_->data->current_score;
    auto &scores = context_->data->high_scores;

    scores.push_back(current_score);
    std::sort(scores.begin(), scores.end(), std::greater<int>());

    if (scores.size() > 5)
    {
        scores.resize(5);
    }

    std::ofstream file("data/highscore.bin", std::ios::binary);
    if (file.is_open())
    {
        file.write(reinterpret_cast<const char *>(scores.data()), 5 * sizeof(int));
        file.close();
    }
}

void GameOverState::setupUI(const sf::Font &font)
{
    background_overlay_.setSize({static_cast<float>(Constants::WINDOW_WIDTH),
                                 static_cast<float>(Constants::WINDOW_HEIGHT)});
    background_overlay_.setFillColor(sf::Color(0, 0, 0, 180)); // 180/255 độ tối

    title_text_->setString("GAME OVER");
    title_text_->setCharacterSize(110);
    title_text_->setFillColor(sf::Color::Red);
    sf::FloatRect title_bounds = title_text_->getLocalBounds();
    title_text_->setPosition({(Constants::WINDOW_WIDTH - title_bounds.size.x) / 2.f, 220.f});

    score_text_->setString("YOUR SCORE: " + std::to_string(context_->data->current_score));
    score_text_->setCharacterSize(55);
    score_text_->setFillColor(sf::Color::Yellow);
    sf::FloatRect score_bounds = score_text_->getLocalBounds();
    score_text_->setPosition({(Constants::WINDOW_WIDTH - score_bounds.size.x) / 2.f, 360.f});

    std::vector<std::string> options = {"TRY AGAIN", "MAIN MENU"};
    float initial_y = 520.f;
    float spacing = 85.f;

    menu_texts_.clear();
    for (size_t i = 0; i < options.size(); ++i)
    {
        auto text = std::make_unique<sf::Text>(font);
        text->setString(options[i]);
        text->setCharacterSize(50);

        if (static_cast<int>(i) == selected_index_)
            text->setFillColor(sf::Color::Yellow);
        else
            text->setFillColor(sf::Color::White);

        sf::FloatRect bounds = text->getLocalBounds();
        text->setPosition({(Constants::WINDOW_WIDTH - bounds.size.x) / 2.f, initial_y + i * spacing});
        menu_texts_.push_back(std::move(text));
    }
}

void GameOverState::handleEvent(const sf::Event &event)
{
    if (const auto *key_pressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (key_pressed->code == sf::Keyboard::Key::Up || key_pressed->code == sf::Keyboard::Key::W)
        {
            moveUp();
        }
        else if (key_pressed->code == sf::Keyboard::Key::Down || key_pressed->code == sf::Keyboard::Key::S)
        {
            moveDown();
        }
        else if (key_pressed->code == sf::Keyboard::Key::Enter)
        {
            if (selected_index_ == 0)
            {
                // Vũ khí tối thượng: Xóa sạch cả PlayingState cũ lẫn GameOverState, nạp màn chơi mới!
                context_->state_machine->replaceRootState(std::make_unique<PlayingState>(context_));
            }
            else
            {
                // Về thẳng màn hình chính
                context_->state_machine->replaceRootState(std::make_unique<HomeState>(context_));
            }
        }
    }
}

void GameOverState::update(float delta_time) {}

void GameOverState::render(sf::RenderWindow &window)
{
    window.draw(background_overlay_);
    window.draw(*title_text_);
    window.draw(*score_text_);
    for (const auto &text : menu_texts_)
    {
        window.draw(*text);
    }
}

void GameOverState::moveUp()
{
    if (selected_index_ > 0)
    {
        menu_texts_[selected_index_]->setFillColor(sf::Color::White);
        selected_index_--;
        menu_texts_[selected_index_]->setFillColor(sf::Color::Yellow);
    }
}

void GameOverState::moveDown()
{
    if (selected_index_ + 1 < static_cast<int>(menu_texts_.size()))
    {
        menu_texts_[selected_index_]->setFillColor(sf::Color::White);
        selected_index_++;
        menu_texts_[selected_index_]->setFillColor(sf::Color::Yellow);
    }
}