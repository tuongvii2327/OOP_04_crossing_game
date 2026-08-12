#include "states/shop_state.h"
#include "utils/audio_system.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"

ShopState::ShopState(SharedContext* context)
	: State(context),
	  selected_index_(0),
	  viewing_char_index_(0),
	  char_sprite_(Resource().getDynamicObjectTexture(Constants::DynamicObjectType::People,
		  Constants::ObjectColor::None))
{
}

void ShopState::init()
{
	const sf::Texture &bg_tex = Resource().getTexture("assets/textures/ui/shop.png");
	background_sprite_ = std::make_unique<sf::Sprite>(bg_tex);
	background_sprite_->setTexture(bg_tex, true);
	background_sprite_->setTextureRect(
		sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(static_cast<int>(bg_tex.getSize().x),
											static_cast<int>(bg_tex.getSize().y))));
	background_sprite_->setPosition({0.f, 0.f});

	float scale_x = static_cast<float>(Constants::WINDOW_WIDTH) / bg_tex.getSize().x;
	float scale_y = static_cast<float>(Constants::WINDOW_HEIGHT) / bg_tex.getSize().y;
	background_sprite_->setScale({scale_x, scale_y});

	const sf::Font &font = Resource().getFont("assets/fonts/VT323-Regular.ttf");
	title_text_ = std::make_unique<sf::Text>(font);
	coin_text_ = std::make_unique<sf::Text>(font);

	// Coin icon (top-left, giống HomeState)
	const sf::Texture &coin_tex = Resource().getTexture("assets/textures/icon/Coin.png");
	coin_sprite_ = std::make_unique<sf::Sprite>(coin_tex);
	coin_sprite_->setTexture(coin_tex, true);
	coin_sprite_->setOrigin({coin_tex.getSize().x / 2.f, coin_tex.getSize().y / 2.f});
	coin_sprite_->setPosition({40.f, 45.f});
	coin_sprite_->setScale({3.5f, 3.5f});

	viewing_char_index_ = context_->data->selected_character;

	setupUI(font);
}

void ShopState::setupUI(const sf::Font &font)
{
	// Tiêu đề Shop
	title_text_->setString("CHARACTER SHOP");
	title_text_->setCharacterSize(72);
	title_text_->setFillColor(sf::Color::Yellow);
	sf::FloatRect title_bounds = title_text_->getLocalBounds();
	title_text_->setPosition({(Constants::WINDOW_WIDTH - title_bounds.size.x) / 2.f, 120.f});

	// Coin text: chỉ số, top-left (giống HomeState)
	coin_text_->setCharacterSize(48);
	coin_text_->setFillColor(sf::Color::Yellow);
	coin_text_->setOutlineColor(sf::Color::Black);
	coin_text_->setOutlineThickness(3.f);

	// Mũi tên đổi nhân vật
	arrow_left_ = std::make_unique<sf::Text>(font);
	arrow_right_ = std::make_unique<sf::Text>(font);
	for (auto* arr : {arrow_left_.get(), arrow_right_.get()})
	{
		arr->setCharacterSize(90);
		arr->setFillColor(sf::Color::White);
		arr->setOutlineColor(sf::Color::Black);
		arr->setOutlineThickness(3.f);
	}
	arrow_left_->setString("<");
	arrow_right_->setString(">");

	// 2 dòng menu: Action (index 0), Back (index 1)
	for (int i = 0; i < 2; ++i)
	{
		menu_texts_.push_back(std::make_unique<sf::Text>(font));
	}

	updateUI();
}

void ShopState::updateUI()
{
	// --- Coin: top-left giống HomeState ---
	coin_text_->setString(std::to_string(context_->data->total_coins));
	{
		sf::FloatRect r = coin_text_->getLocalBounds();
		coin_text_->setOrigin({0.f, r.size.y / 2.f});
		coin_text_->setPosition({70.f, 40.f});
	}

	// --- Ảnh nhân vật ---
	Constants::ObjectColor preview_color = Constants::ObjectColor::None;
	switch (viewing_char_index_)
	{
	case 1:
		preview_color = Constants::ObjectColor::Blue;
		break;
	case 2:
		preview_color = Constants::ObjectColor::Green;
		break;
	}

	const sf::Texture &tex =
		Resource().getDynamicObjectTexture(Constants::DynamicObjectType::People, preview_color);
	char_sprite_.setTexture(tex, true);
	char_sprite_.setTextureRect(sf::IntRect(sf::Vector2i(0, 0),
		sf::Vector2i(Constants::SPRITESHEET_FRAME_SIZE,
			Constants::SPRITESHEET_FRAME_SIZE)));

	float preview_scale = 0.32f;
	char_sprite_.setScale({preview_scale, preview_scale});

	// Khoảng hiển thị nhân vật: y = 280 đến 570 (290px)
	sf::FloatRect sprite_bounds = char_sprite_.getGlobalBounds();
	float sprite_x = (Constants::WINDOW_WIDTH - sprite_bounds.size.x) / 2.f;
	float sprite_y = 280.f + (290.f - sprite_bounds.size.y) / 2.f;
	char_sprite_.setPosition({sprite_x, sprite_y});

	// --- Mũi tên hai bên nhân vật ---
	const float arrow_margin = 30.f;
	float sprite_center_y = sprite_y + sprite_bounds.size.y / 2.f;

	{
		sf::FloatRect lr = arrow_left_->getLocalBounds();
		arrow_left_->setOrigin({lr.size.x, lr.size.y / 2.f});
		arrow_left_->setPosition({sprite_x - arrow_margin, sprite_center_y});
	}
	{
		sf::FloatRect rr = arrow_right_->getLocalBounds();
		arrow_right_->setOrigin({0.f, rr.size.y / 2.f});
		arrow_right_->setPosition(
			{sprite_x + sprite_bounds.size.x + arrow_margin, sprite_center_y});
	}

	// --- Menu: Action (index 0), Back (index 1) ---
	std::string action_str;
	if (!context_->data->unlocked_characters[viewing_char_index_])
	{
		action_str = "[ BUY - " + std::to_string(char_prices_[viewing_char_index_]) + " COINS ]";
	}
	else if (context_->data->selected_character == viewing_char_index_)
	{
		action_str = "[ EQUIPPED ]";
	}
	else
	{
		action_str = "[ EQUIP ]";
	}

	std::vector<std::string> display_strings = {action_str, "BACK TO MENU"};
	float initial_y = 570.f;
	float spacing = 90.f;

	for (size_t i = 0; i < menu_texts_.size(); ++i)
	{
		menu_texts_[i]->setString(display_strings[i]);
		menu_texts_[i]->setCharacterSize(55);
		menu_texts_[i]->setFillColor(static_cast<int>(i) == selected_index_ ? sf::Color::Yellow
																			: sf::Color::White);
		sf::FloatRect tb = menu_texts_[i]->getLocalBounds();
		float x = (Constants::WINDOW_WIDTH - tb.size.x) / 2.f;
		float y = initial_y + static_cast<float>(i) * spacing;
		menu_texts_[i]->setPosition({x, y});
	}
}

void ShopState::handleEvent(const sf::Event &event)
{
	if (const auto* key_pressed = event.getIf<sf::Event::KeyPressed>())
	{
		if (key_pressed->code == sf::Keyboard::Key::Up || key_pressed->code == sf::Keyboard::Key::W)
		{
			moveUp();
			Audio().playSFX(AudioSystem::SFX::Hover);
		}
		else if (key_pressed->code == sf::Keyboard::Key::Down ||
				 key_pressed->code == sf::Keyboard::Key::S)
		{
			moveDown();
			Audio().playSFX(AudioSystem::SFX::Hover);
		}
		else if (key_pressed->code == sf::Keyboard::Key::Left ||
				 key_pressed->code == sf::Keyboard::Key::A)
		{
			prevCharacter();
			Audio().playSFX(AudioSystem::SFX::Hover);
		}
		else if (key_pressed->code == sf::Keyboard::Key::Right ||
				 key_pressed->code == sf::Keyboard::Key::D)
		{
			nextCharacter();
			Audio().playSFX(AudioSystem::SFX::Hover);
		}
		else if (key_pressed->code == sf::Keyboard::Key::Enter)
		{
			executeAction();
		}
		else if (key_pressed->code == sf::Keyboard::Key::Escape)
		{
			Audio().playSFX(AudioSystem::SFX::Back);
			context_->state_machine->pop();
		}
	}

	// Click chuột vào mũi tên hoặc menu
	if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>())
	{
		if (mb->button == sf::Mouse::Button::Left)
		{
			sf::Vector2f mp(static_cast<float>(mb->position.x), static_cast<float>(mb->position.y));

			if (arrow_left_ && arrow_left_->getGlobalBounds().contains(mp))
			{
				Audio().playSFX(AudioSystem::SFX::Hover);
				prevCharacter();
			}
			else if (arrow_right_ && arrow_right_->getGlobalBounds().contains(mp))
			{
				Audio().playSFX(AudioSystem::SFX::Hover);
				nextCharacter();
			}
			else
			{
				// Click trên menu text
				for (size_t i = 0; i < menu_texts_.size(); ++i)
				{
					if (menu_texts_[i] && menu_texts_[i]->getGlobalBounds().contains(mp))
					{
						selected_index_ = static_cast<int>(i);
						Audio().playSFX(AudioSystem::SFX::Click);
						executeAction();
						break;
					}
				}
			}
		}
	}
}

void ShopState::update(float delta_time)
{
}

void ShopState::render(sf::RenderWindow &window)
{
	window.clear(sf::Color(30, 30, 45));

	if (background_sprite_)
		window.draw(*background_sprite_);

	// Coin icon + số xu (top-left)
	if (coin_sprite_)
		window.draw(*coin_sprite_);
	if (coin_text_)
		window.draw(*coin_text_);

	// Nhân vật và mũi tên
	window.draw(char_sprite_);
	if (arrow_left_)
		window.draw(*arrow_left_);
	if (arrow_right_)
		window.draw(*arrow_right_);

	// Menu text
	for (const auto &text : menu_texts_)
	{
		if (text)
			window.draw(*text);
	}
}

void ShopState::moveUp()
{
	selected_index_ = (selected_index_ - 1 + static_cast<int>(menu_texts_.size())) %
					  static_cast<int>(menu_texts_.size());
	updateUI();
}

void ShopState::moveDown()
{
	selected_index_ = (selected_index_ + 1) % static_cast<int>(menu_texts_.size());
	updateUI();
}

void ShopState::nextCharacter()
{
	viewing_char_index_ = (viewing_char_index_ + 1) % NUM_CHARS;
	updateUI();
}

void ShopState::prevCharacter()
{
	viewing_char_index_ = (viewing_char_index_ - 1 + NUM_CHARS) % NUM_CHARS;
	updateUI();
}

void ShopState::executeAction()
{
	if (selected_index_ == 1) // Back
	{
		Audio().playSFX(AudioSystem::SFX::Back);
		context_->state_machine->pop();
	}
	else if (selected_index_ == 0) // Action: Buy / Equip
	{
		bool is_unlocked = context_->data->unlocked_characters[viewing_char_index_];
		if (!is_unlocked)
		{
			int price = char_prices_[viewing_char_index_];
			if (context_->data->total_coins >= price)
			{
				context_->data->total_coins -= price;
				context_->data->unlocked_characters[viewing_char_index_] = true;
				context_->data->selected_character = viewing_char_index_;
				Audio().playSFX(AudioSystem::SFX::Click);
			}
			else
			{
				Audio().playSFX(AudioSystem::SFX::Crash);
			}
		}
		else
		{
			if (context_->data->selected_character != viewing_char_index_)
			{
				context_->data->selected_character = viewing_char_index_;
				Audio().playSFX(AudioSystem::SFX::Click);
			}
		}
		updateUI();
	}
}