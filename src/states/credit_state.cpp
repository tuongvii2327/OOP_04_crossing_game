#include "states/credit_state.h"
#include "utils/audio_system.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"
#include <fstream>
#include <iostream>
#include <algorithm>

CreditState::CreditState(SharedContext* context) : State(context) {}

void CreditState::init()
{
    const sf::Font& font = Resource().getFont("assets/fonts/VT323-Regular.ttf");
    title_text_ = std::make_unique<sf::Text>(font);
    back_btn_ = std::make_unique<sf::Text>(font);
    setupUI(font);
}

void CreditState::setupUI(const sf::Font& font)
{
    title_text_->setString("DEVELOPMENT TEAM");
    title_text_->setCharacterSize(64);
    title_text_->setFillColor(sf::Color::Magenta);
    sf::FloatRect title_bounds = title_text_->getLocalBounds();
    title_text_->setPosition({(Constants::WINDOW_WIDTH - title_bounds.size.x) / 2.0f, 70.0f});

    std::vector<std::string> lines;
    std::ifstream file("data/credit.txt");
    if (file.is_open())
    {
        std::string l;
        while (std::getline(file, l)) lines.push_back(l);
        file.close();
    }
    else { lines.push_back("Cannot open data/credit.txt"); }

    float start_y = 160.0f;
    float max_height = Constants::WINDOW_HEIGHT - 280.0f;
    float spacing = 38.0f;
    if (!lines.empty() && (lines.size() * spacing > max_height))
    {
        spacing = max_height / static_cast<float>(lines.size());
    }

    int font_size = std::min(32, static_cast<int>(spacing * 0.85f));

    float max_line_width = 0.f;
    for (const auto& line_str : lines)
    {
        sf::Text temp_text(font);
        temp_text.setString(line_str);
        temp_text.setCharacterSize(font_size);
        float w = temp_text.getLocalBounds().size.x;
        if (w > max_line_width) max_line_width = w;
    }

    float left_margin = (Constants::WINDOW_WIDTH - max_line_width) / 2.0f;

    lines_text_.clear();
    for (size_t i = 0; i < lines.size(); ++i)
    {
        auto t = std::make_unique<sf::Text>(font);
        t->setString(lines[i]);
        t->setCharacterSize(font_size);
        
        if (lines[i].find("---") != std::string::npos)
            t->setFillColor(sf::Color::Yellow);
        else
            t->setFillColor(sf::Color::White);

        t->setPosition({left_margin, start_y + i * spacing});
        lines_text_.push_back(std::move(t));
    }

    back_btn_->setString("BACK");
    back_btn_->setCharacterSize(48);
    back_btn_->setFillColor(sf::Color::White);
    sf::FloatRect back_bounds = back_btn_->getLocalBounds();
    back_btn_->setPosition({(Constants::WINDOW_WIDTH - back_bounds.size.x) / 2.0f, Constants::WINDOW_HEIGHT - 80.0f});
}

void CreditState::handleEvent(const sf::Event& event)
{
    if (const auto* kp = event.getIf<sf::Event::KeyPressed>())
    {
        if (kp->code == sf::Keyboard::Key::Escape || kp->code == sf::Keyboard::Key::Enter)
        {
            Audio().playSFX(AudioSystem::SFX::Back);
            context_->state_machine->pop();
        }
    }

    if (const auto* mm = event.getIf<sf::Event::MouseMoved>())
    {
        sf::Vector2f mp = context_->window->mapPixelToCoords(mm->position);
        bool hover = back_btn_->getGlobalBounds().contains(mp);
        if (hover != is_back_hovered_)
        {
            is_back_hovered_ = hover;
            back_btn_->setFillColor(hover ? sf::Color::Yellow : sf::Color::White);
            if (hover) Audio().playSFX(AudioSystem::SFX::Hover);
        }
    }

    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mb->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mp = context_->window->mapPixelToCoords(mb->position);
            if (back_btn_->getGlobalBounds().contains(mp))
            {
                Audio().playSFX(AudioSystem::SFX::Back);
                context_->state_machine->pop();
            }
        }
    }
}

void CreditState::update(float delta_time) {}

void CreditState::render(sf::RenderWindow& window)
{
    window.clear(sf::Color(25, 25, 30));
    if (title_text_) window.draw(*title_text_);
    for (const auto& line : lines_text_) if (line) window.draw(*line);
    if (back_btn_) window.draw(*back_btn_);
}