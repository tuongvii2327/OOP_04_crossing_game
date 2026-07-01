#include "states/ranking_state.h"
#include "utils/constants.h"
#include <iostream>

RankingState::RankingState(SharedContext* context)
	: State(context)
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

	std::vector<std::string> rankings;
	for (size_t i = 0; i < 5; ++i)
	{
		int score = context_->data->high_scores[i];
		rankings.push_back("TOP " + std::to_string(i + 1) + ".............. " + std::to_string(score));
	}

	rankings.push_back("");
	rankings.push_back("Press ESC to go back");

	float initial_y = 300.0f;
	float spacing = 60.0f;

	ranking_texts_.clear();
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
	if (const auto* key_pressed = event.getIf<sf::Event::KeyPressed>())
	{
		if (key_pressed->code == sf::Keyboard::Key::Escape)
		{
			context_->state_machine->pop();
		}
	}
}

// Update ranking state (static)
void RankingState::update(float delta_time) {}

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
