#include "states/help_state.h"
#include "utils/audio_system.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"
#include <fstream>
#include <iostream>
#include <algorithm>

HelpState::HelpState(SharedContext* context) : State(context) {}

void HelpState::init()
{
    const sf::Font& font = Resource().getFont("assets/fonts/VT323-Regular.ttf");
    title_text_ = std::make_unique<sf::Text>(font);
    back_btn_ = std::make_unique<sf::Text>(font);
    setupUI(font);
}

void HelpState::setupUI(const sf::Font& font)
{
    // 1. TIÊU ĐỀ
    title_text_->setString("HOW TO PLAY");
    title_text_->setCharacterSize(64);
    title_text_->setFillColor(sf::Color::Cyan);
    sf::FloatRect title_bounds = title_text_->getLocalBounds();
    title_text_->setPosition({(Constants::WINDOW_WIDTH - title_bounds.size.x) / 2.0f, 70.0f});

    // 2. NẠP FILE TXT
    std::vector<std::string> lines;
    std::ifstream file("data/help.txt");
    if (file.is_open())
    {
        std::string l;
        while (std::getline(file, l)) lines.push_back(l);
        file.close();
    }
    else { lines.push_back("Cannot open data/help.txt"); }

    // 3. TÍNH SPACING VÀ ĐỘ RỘNG CỦA DÒNG DÀI NHẤT
    float start_y = 160.0f;
    float max_height = Constants::WINDOW_HEIGHT - 280.0f;
    float spacing = 38.0f;
    if (!lines.empty() && (lines.size() * spacing > max_height))
    {
        spacing = max_height / static_cast<float>(lines.size());
    }

    int font_size = std::min(32, static_cast<int>(spacing * 0.85f));

    // Tìm độ rộng dòng dài nhất để tính Lề Trái
    float max_line_width = 0.f;
    for (const auto& line_str : lines)
    {
        sf::Text temp_text(font);
        temp_text.setString(line_str);
        temp_text.setCharacterSize(font_size);
        float w = temp_text.getLocalBounds().size.x;
        if (w > max_line_width) max_line_width = w;
    }

    // Lề trái chung cho TOÀN BỘ KHỐI VĂN BẢN
    float left_margin = (Constants::WINDOW_WIDTH - max_line_width) / 2.0f;

    // 4. TẠO TEXT VỚI LỀ TRÁI CỐ ĐỊNH THẲNG HÀNG
    lines_text_.clear();
    for (size_t i = 0; i < lines.size(); ++i)
    {
        auto t = std::make_unique<sf::Text>(font);
        t->setString(lines[i]);
        t->setCharacterSize(font_size);
        
        // Đổi màu vàng cho các dòng tiêu đề phụ (dòng có dấu ---)
        if (lines[i].find("---") != std::string::npos)
            t->setFillColor(sf::Color::Yellow);
        else
            t->setFillColor(sf::Color::White);

        // Đặt X bằng đúng lề trái chung
        t->setPosition({left_margin, start_y + i * spacing});
        lines_text_.push_back(std::move(t));
    }

    // 5. NÚT BACK TO MENU CỐ ĐỊNH Ở ĐÁY MÀN HÌNH
    back_btn_->setString("BACK TO MENU");
    back_btn_->setCharacterSize(48);
    back_btn_->setFillColor(sf::Color::White);
    sf::FloatRect back_bounds = back_btn_->getLocalBounds();
    back_btn_->setPosition({(Constants::WINDOW_WIDTH - back_bounds.size.x) / 2.0f, Constants::WINDOW_HEIGHT - 80.0f});
}

void HelpState::handleEvent(const sf::Event& event)
{
    // Bàn phím: ESC hoặc Enter
    if (const auto* kp = event.getIf<sf::Event::KeyPressed>())
    {
        if (kp->code == sf::Keyboard::Key::Escape || kp->code == sf::Keyboard::Key::Enter)
        {
            Audio().playSFX(AudioSystem::SFX::Back);
            context_->state_machine->pop();
        }
    }

    // Di chuột Hover nút Back cố định
    if (const auto* mm = event.getIf<sf::Event::MouseMoved>())
    {
        sf::Vector2f mp = context_->window->mapPixelToCoords(mm->position); // Đồng bộ tọa độ chuẩn SFML
        bool hover = back_btn_->getGlobalBounds().contains(mp);
        if (hover != is_back_hovered_)
        {
            is_back_hovered_ = hover;
            back_btn_->setFillColor(hover ? sf::Color::Yellow : sf::Color::White);
            if (hover) Audio().playSFX(AudioSystem::SFX::Hover);
        }
    }

    // Click chuột vào nút Back cố định
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

void HelpState::update(float delta_time) {}

void HelpState::render(sf::RenderWindow& window)
{
    window.clear(sf::Color(25, 25, 30));
    if (title_text_) window.draw(*title_text_);
    for (const auto& line : lines_text_) if (line) window.draw(*line);
    if (back_btn_) window.draw(*back_btn_); // Luôn vẽ nút Back cố định
}