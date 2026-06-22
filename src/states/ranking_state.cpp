#include "states/ranking_state.h"
#include "utils/constants.h"
#include <iostream>

RankingState::RankingState(StateMachine &state_machine)
    : state_machine_(state_machine)
{
}

// Load font and initialize ranking screen
void RankingState::init()
{
    const sf::Font &font = Resource().getFont("assets/fonts/VT323-Regular.ttf");
    title_text_ = std::make_unique<sf::Text>(font);
    setupUI(font);
}

// Setup ranking display UI with scores
void RankingState::setupUI(const sf::Font &font)
{
    title_text_->setString("HIGH SCORES");
    title_text_->setCharacterSize(72);
    title_text_->setFillColor(sf::Color::Red);

    sf::FloatRect title_bounds = title_text_->getLocalBounds();
    float title_x = (Constants::WINDOW_WIDTH - title_bounds.size.x) / 2.0f;
    title_text_->setPosition({title_x, 150.0f});

    std::vector<std::string> rankings = {
        "1. PLAYER 1 ............ 9999",
        "2. PLAYER 2 ............ 8500",
        "3. PLAYER 3 ............ 7200",
        "4. PLAYER 4 ............ 6100",
        "5. PLAYER 5 ............ 5000",
        "",
        "Press ESC to go back"};

    float initial_y = 300.0f;
    float spacing = 60.0f;

    for (size_t i = 0; i < rankings.size(); ++i)
    {
        auto ranking_text = std::make_unique<sf::Text>(font);
        ranking_text->setString(rankings[i]);
        ranking_text->setCharacterSize(40);
        ranking_text->setFillColor(sf::Color::White);

        sf::FloatRect text_bounds = ranking_text->getLocalBounds();
        float x_position = (Constants::WINDOW_WIDTH - text_bounds.size.x) / 2.0f;
        float y_position = initial_y + (static_cast<float>(i) * spacing);

        ranking_text->setPosition({x_position, y_position});
        ranking_texts_.push_back(std::move(ranking_text));
    }
}

// Handle escape key input
void RankingState::handleEvent(const sf::Event &event)
{
    if (const auto *key_pressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (key_pressed->code == sf::Keyboard::Key::Escape)
        {
            state_machine_.removeState();
        }
    }
}

// Update ranking state (static)
void RankingState::update(float delta_time)
{
}

// Render ranking display
void RankingState::render(sf::RenderWindow &window)
{
    window.clear(sf::Color(20, 20, 40));

    if (title_text_)
    {
        window.draw(*title_text_);
    }

    for (const auto &ranking_text : ranking_texts_)
    {
        if (ranking_text)
        {
            window.draw(*ranking_text);
        }
    }
}