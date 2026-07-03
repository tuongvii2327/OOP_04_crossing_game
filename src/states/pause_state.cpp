#include "states/pause_state.h"
#include "states/home_state.h"
#include "states/save_state.h"
#include "states/setting_state.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"
#include <iostream>

PauseState::PauseState(SharedContext* context)
	: State(context), selected_index_(0)
{
}

// Load font and initialize UI
void PauseState::init()
{
	const sf::Font &font = Resource().getFont("assets/fonts/VT323-Regular.ttf");
	title_text_ = std::make_unique<sf::Text>(font);

	setupUI(font);
}

// Setup pause menu UI with overlay and options
void PauseState::setupUI(const sf::Font &font)
{
	background_overlay_.setSize({static_cast<float>(Constants::WINDOW_WIDTH),
	                             static_cast<float>(Constants::WINDOW_HEIGHT)});
	background_overlay_.setFillColor(sf::Color(0, 0, 0, 150));

	title_text_->setString("PAUSED");
	title_text_->setCharacterSize(90);
	title_text_->setFillColor(sf::Color::White);

	sf::FloatRect title_bounds = title_text_->getLocalBounds();
	float title_x = (Constants::WINDOW_WIDTH - title_bounds.size.x) / 2.0f;
	title_text_->setPosition({title_x, 200.0f});

	std::vector<std::string> options = {"RESUME", "SAVE GAME", "SETTINGS", "HOME"};
	float initial_y = 450.0f;
	float spacing = 80.0f;

	for (size_t i = 0; i < options.size(); ++i)
	{
		std::unique_ptr<sf::Text> option_text = std::make_unique<sf::Text>(font);
		option_text->setString(options[i]);
		option_text->setCharacterSize(50);

		if (static_cast<int>(i) == selected_index_)
		{
			option_text->setFillColor(sf::Color::Yellow);
		}
		else
		{
			option_text->setFillColor(sf::Color::White);
		}

		sf::FloatRect text_bounds = option_text->getLocalBounds();
		float x_position = (Constants::WINDOW_WIDTH - text_bounds.size.x) / 2.0f;
		float y_position = initial_y + (static_cast<float>(i) * spacing);

		option_text->setPosition({x_position, y_position});
		menu_texts_.push_back(std::move(option_text));
	}
}

// Handle pause menu input
void PauseState::handleEvent(const sf::Event &event)
{
	if (const auto* key_pressed = event.getIf<sf::Event::KeyPressed>())
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
				context_->state_machine->pop(); // Resume
			}
			else if (selected_index_ == 1)
			{
				context_->state_machine->push(std::make_unique<SaveState>(context_));
			}
			else if (selected_index_ == 2)
			{
				context_->state_machine->push(std::make_unique<SettingState>(context_));
			}
			else if (selected_index_ == 3)
			{
				context_->state_machine->resetTo(std::make_unique<HomeState>(context_));
			}
		}
		else if (key_pressed->code == sf::Keyboard::Key::Escape)
		{
			context_->state_machine->pop(); // Resume
		}
	}
}

// Update pause state (static)
void PauseState::update(float delta_time) {}

// Render pause menu overlay and options
void PauseState::render(sf::RenderWindow &window)
{
	window.draw(background_overlay_);
	if (title_text_)
	{
		window.draw(*title_text_);
	}
	for (const auto &menu_text : menu_texts_)
	{
		if (menu_text)
		{
			window.draw(*menu_text);
		}
	}
}

// Move selection up
void PauseState::moveUp()
{
	if (selected_index_ - 1 >= 0)
	{
		menu_texts_[selected_index_]->setFillColor(sf::Color::White);
		selected_index_--;
		menu_texts_[selected_index_]->setFillColor(sf::Color::Yellow);
	}
}

// Move selection down
void PauseState::moveDown()
{
	if (selected_index_ + 1 < static_cast<int>(menu_texts_.size()))
	{
		menu_texts_[selected_index_]->setFillColor(sf::Color::White);
		selected_index_++;
		menu_texts_[selected_index_]->setFillColor(sf::Color::Yellow);
	}
}
