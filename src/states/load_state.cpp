// src/states/load_state.cpp
#include "states/load_state.h"
#include "states/playing_state.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"
#include <iostream>

LoadState::LoadState(SharedContext* context)
	: State(context), selected_index_(0)
{
}

// Load font and initialize load screen
void LoadState::init()
{
	const sf::Font &font = Resource().getFont("assets/fonts/VT323-Regular.ttf");
	title_text_ = std::make_unique<sf::Text>(font);
	setupUI(font);
}

// Setup load menu UI with save slots
void LoadState::setupUI(const sf::Font &font)
{
	title_text_->setString("LOAD GAME");
	title_text_->setCharacterSize(72);
	title_text_->setFillColor(sf::Color::Cyan);

	sf::FloatRect title_bounds = title_text_->getLocalBounds();
	float title_x = (Constants::WINDOW_WIDTH - title_bounds.size.x) / 2.0f;
	title_text_->setPosition({title_x, 150.0f});

	std::vector<std::string> save_options = {"SAVE SLOT 1", "SAVE SLOT 2",
	                                         "SAVE SLOT 3", "BACK TO MENU"};

	float initial_y = 350.0f;
	float spacing = 80.0f;

	save_slot_texts_.clear();
	for (size_t i = 0; i < save_options.size(); ++i)
	{
		sf::Text slot_text(font);
		slot_text.setString(save_options[i]);
		slot_text.setCharacterSize(48);

		if (static_cast<int>(i) == selected_index_)
		{
			slot_text.setFillColor(sf::Color::Yellow);
		}
		else
		{
			slot_text.setFillColor(sf::Color::White);
		}

		sf::FloatRect text_bounds = slot_text.getLocalBounds();
		float x_position =
			(Constants::WINDOW_WIDTH - text_bounds.size.x) / 2.0f;
		float y_position = initial_y + (static_cast<float>(i) * spacing);

		slot_text.setPosition({x_position, y_position});
		save_slot_texts_.push_back(std::move(slot_text));
	}
}

// Handle slot selection and navigation
void LoadState::handleEvent(const sf::Event &event)
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
			if (selected_index_ == static_cast<int>(save_slot_texts_.size()) - 1)
			{
				context_->state_machine->pop(); // Back to menu
			}
			else
			{
				loadGameData(selected_index_ + 1);
			}
		}
		else if (key_pressed->code == sf::Keyboard::Key::Escape)
		{
			context_->state_machine->pop();
		}
	}
}

// Update load state (static)
void LoadState::update(float delta_time) {}

// Render load menu
void LoadState::render(sf::RenderWindow &window)
{
	window.clear(sf::Color(25, 25, 25));

	if (title_text_)
	{
		window.draw(*title_text_);
	}

	for (const auto &slot_text : save_slot_texts_)
	{
		window.draw(slot_text);
	}
}

// Move selection up
void LoadState::moveUp()
{
	if (selected_index_ - 1 >= 0)
	{
		save_slot_texts_[selected_index_].setFillColor(sf::Color::White);
		selected_index_--;
		save_slot_texts_[selected_index_].setFillColor(sf::Color::Yellow);
	}
}

// Move selection down
void LoadState::moveDown()
{
	if (selected_index_ + 1 < static_cast<int>(save_slot_texts_.size()))
	{
		save_slot_texts_[selected_index_].setFillColor(sf::Color::White);
		selected_index_++;
		save_slot_texts_[selected_index_].setFillColor(sf::Color::Yellow);
	}
}

// Load game data from save file
void LoadState::loadGameData(int slot_index)
{
	context_->state_machine->switchState(std::make_unique<PlayingState>(context_));
}
