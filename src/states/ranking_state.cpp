#include "states/ranking_state.h"
#include "utils/audio_system.h"
#include "utils/constants.h"

RankingState::RankingState(SharedContext* context) : State(context) {}

// Load font and initialize ranking screen
void RankingState::init()
{
    const sf::Texture& bg_tex = Resource().getTexture("assets/textures/ui/ranking.png");
    background_sprite_ = std::make_unique<sf::Sprite>(bg_tex);
    background_sprite_->setTexture(bg_tex, true);
    background_sprite_->setTextureRect(
        sf::IntRect({0, 0}, sf::Vector2i(static_cast<int>(bg_tex.getSize().x),
                                         static_cast<int>(bg_tex.getSize().y))));
    background_sprite_->setPosition({0.f, 0.f});

    float scale_x =
        static_cast<float>(Constants::WINDOW_WIDTH) / static_cast<float>(bg_tex.getSize().x);
    float scale_y =
        static_cast<float>(Constants::WINDOW_HEIGHT) / static_cast<float>(bg_tex.getSize().y);
    background_sprite_->setScale({scale_x, scale_y});

    const sf::Font& font = Resource().getFont("assets/fonts/VT323-Regular.ttf");
    title_text_ = std::make_unique<sf::Text>(font);
    setupUI();
}

// Setup ranking display UI with scores
void RankingState::setupUI()
{
    *title_text_ = Resource().createText("HIGH SCORES", {72, sf::Color::Red});
    sf::FloatRect title_bounds = title_text_->getLocalBounds();
    title_text_->setPosition({Constants::WINDOW_WIDTH / 2.0f, 150.0f + title_bounds.size.y / 2.0f});

    std::vector<std::string> rankings;
    for (size_t i = 0; i < 5; ++i)
    {
        int score = context_->data->high_scores[i];
        rankings.push_back("TOP " + std::to_string(i + 1) + ".............. " +
                           std::to_string(score));
    }

    rankings.push_back("");
    rankings.push_back("BACK TO MENU");

    float initial_y = 350.0f;
    float spacing = 80.0f;

    ranking_texts_.clear();
    for (size_t i = 0; i < rankings.size(); ++i)
    {
        auto ranking_text = std::make_unique<sf::Text>(Resource().createText(rankings[i], {60}));

        sf::FloatRect text_bounds = ranking_text->getLocalBounds();
        float x_position = Constants::WINDOW_WIDTH / 2.0f;
        float y_position = initial_y + (static_cast<float>(i) * spacing) + text_bounds.size.y / 2.0f;

        ranking_text->setPosition({x_position, y_position});
        ranking_texts_.push_back(std::move(ranking_text));
    }
}

// Handle escape key input
void RankingState::handleEvent(const sf::Event& event)
{
    // 1. Xử lý bàn phím: ESC hoặc Enter để quay về
    if (const auto* key_pressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (key_pressed->code == sf::Keyboard::Key::Escape ||
            key_pressed->code == sf::Keyboard::Key::Enter)
        {
            Audio().playSFX(AudioSystem::SFX::Back);
            context_->state_machine->pop();
        }
    }

    // 2. Xử lý di chuột: Hover vào nút "BACK TO MENU" sẽ đổi sang màu Vàng
    if (const auto* mouse_moved = event.getIf<sf::Event::MouseMoved>())
    {
        sf::Vector2f mouse_pos = context_->window->mapPixelToCoords(mouse_moved->position);

        if (!ranking_texts_.empty())
        {
            auto& back_btn = ranking_texts_.back(); // Nút BACK TO MENU ở vị trí cuối vector
            Resource().setTextButtonHovered(*back_btn, back_btn->getGlobalBounds().contains(mouse_pos));
        }
    }

    // 3. Xử lý Click chuột: Click vào "BACK TO MENU" sẽ quay về Menu chính
    if (const auto* mouse_button_pressed = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mouse_button_pressed->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mouse_pos = context_->window->mapPixelToCoords(mouse_button_pressed->position);

            if (!ranking_texts_.empty() && ranking_texts_.back()->getGlobalBounds().contains(mouse_pos))
            {
                Audio().playSFX(AudioSystem::SFX::Back);
                context_->state_machine->pop();
            }
        }
    }
}

// Update ranking state (static)
void RankingState::update(float delta_time) {}

// Render ranking display
void RankingState::render(sf::RenderWindow& window)
{
    window.clear(sf::Color(20, 20, 40));

    if (background_sprite_)
    {
        window.draw(*background_sprite_);
    }

    // if (title_text_)
    // {
    //     window.draw(*title_text_);
    // }

    for (const auto& ranking_text : ranking_texts_)
    {
        if (ranking_text)
        {
            Resource().drawTextWithShadow(window, *ranking_text);
        }
    }
}
