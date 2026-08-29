// src/states/save_state.cpp
#include "states/save_state.h"
#include "../include/utils/serialization.h"
#include "states/playing_state.h"
#include "utils/audio_system.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"
#include <cctype>
#include <fstream>
#include <iostream>

SaveState::SaveState(SharedContext* context)
    : State(context), selected_index_(0), is_saved_(false), save_timer_(0.0f)
{
}

// Load font and initialize save screen
void SaveState::init()
{
    // Background
    const sf::Texture& bg_tex = Resource().getTexture("assets/textures/ui/save.png");
    background_sprite_ = std::make_unique<sf::Sprite>(bg_tex);
    background_sprite_->setTexture(bg_tex, true);
    background_sprite_->setPosition({0.f, 0.f});
    background_sprite_->setScale(
        {static_cast<float>(Constants::WINDOW_WIDTH) / bg_tex.getSize().x,
         static_cast<float>(Constants::WINDOW_HEIGHT) / bg_tex.getSize().y});

    const sf::Font& font = Resource().getFont("assets/fonts/VT323-Regular.ttf");

    // Khởi tạo các Text hiển thị cho chế độ gõ tên
    prompt_text_ = std::make_unique<sf::Text>(font);
    input_display_text_ = std::make_unique<sf::Text>(font);
    error_text_ = std::make_unique<sf::Text>(font);
    overwrite_prompt_text_ = std::make_unique<sf::Text>(font);
    setupUI();
}

// Setup save menu UI with save slots
void SaveState::setupUI()
{
    save_slot_texts_.clear();

    // (Title removed — background image already contains it)

    float initial_y = 350.0f;
    float spacing = 100.0f;

    save_slot_texts_.clear();

    for (size_t i = 1; i <= 3; ++i)
    {
        std::string slot_name;
        int slot_score = 0;
        std::string display_str;

        if (Serialization::getSlotMetadata(i, slot_name, slot_score))
        {
            display_str = "SLOT " + std::to_string(i) + ": " + slot_name +
                          " - Score: " + std::to_string(slot_score);
        }
        else
        {
            display_str = "SLOT " + std::to_string(i) + ": - EMPTY -";
        }

        auto slot_text = std::make_unique<sf::Text>(Resource().createText(display_str, {70}, true));

        if (i - 1 == selected_index_)
        {
            Resource().setTextButtonHovered(*slot_text, true);
        }

        sf::FloatRect text_bounds = slot_text->getLocalBounds();
        float x_position = Constants::WINDOW_WIDTH / 2.0f;
        float y_position = initial_y + (i - 1) * spacing + text_bounds.size.y / 2.0f;

        slot_text->setPosition({x_position, y_position});
        save_slot_texts_.push_back(std::move(slot_text));
    }

    // NÚT BACK
    auto back_text = std::make_unique<sf::Text>(Resource().createText("BACK", {70}, true));
    if (selected_index_ == 3)
        Resource().setTextButtonHovered(*back_text, true);

    sf::FloatRect back_bounds = back_text->getLocalBounds();
    back_text->setPosition(
        {Constants::WINDOW_WIDTH / 2.0f, initial_y + 3 * spacing + back_bounds.size.y / 2.0f});
    save_slot_texts_.push_back(std::move(back_text));

    // 2. SETUP UI CẢNH BÁO GHI ĐÈ (OVERWRITE PROMPT)
    *overwrite_prompt_text_ =
        Resource().createText("\tSLOT ALREADY EXISTS!\nOVERWRITE SAVE? (ENTER / ESC)", {75, sf::Color::Yellow });
    overwrite_prompt_text_->setOrigin({0.f, 0.f});

    // 3. UI GÕ TÊN
    *prompt_text_ = Resource().createText("ENTER YOU NAME (3 - 15 CHARS):", {60, sf::Color::Yellow });
    prompt_text_->setOrigin({0.f, 0.f});
    sf::FloatRect p_bounds = prompt_text_->getLocalBounds();
    prompt_text_->setPosition({(Constants::WINDOW_WIDTH - p_bounds.size.x) / 2.0f, 250.0f});

    *input_display_text_ = Resource().createText("_", {60, FILL_BUTTON_COLOR });
    input_display_text_->setOrigin({0.f, 0.f});
    input_display_text_->setPosition({100.f, 480.0f});

    *error_text_ = Resource().createText("", {60, sf::Color::Red, sf::Color::White});
    error_text_->setOrigin({0.f, 0.f});
}

void SaveState::handleEvent(const sf::Event& event)
{
    // CONFIRM OVERRIDE
    if (current_mode_ == Mode::ConfirmOverwrite)
    {
        if (const auto* key_pressed = event.getIf<sf::Event::KeyPressed>())
        {
            if (key_pressed->code == sf::Keyboard::Key::Enter)
            {
                // Đồng ý ghi đè -> Chuyển sang chế độ gõ tên
                current_mode_ = Mode::NamingMode;
                input_name_ = "";
                input_display_text_->setString("_");
                error_text_->setString("");
            }
            else if (key_pressed->code == sf::Keyboard::Key::Escape)
            {
                // Từ chối -> Hủy bỏ, quay lại chọn Slot
                current_mode_ = Mode::SelectSlot;
            }
        }
        return;
    }

    // ---- MODE 1: INPUTTING NAME ----
    if (current_mode_ == Mode::NamingMode)
    {
        // 1. Xử lý gõ phím ký tự Unicode
        if (const auto* text_entered = event.getIf<sf::Event::TextEntered>())
        {
            char32_t unicode = text_entered->unicode;

            if (unicode == 8) // BACKSPACE
            {
                if (!input_name_.empty())
                    input_name_.pop_back();

                error_text_->setString("");
            }

            else if (unicode >= 32 && unicode < 127)
            {
                char c = static_cast<char>(unicode);

                // RÀNG BUỘC KÝ TỰ: Chỉ chấp nhận chữ cái, số và khoảng trắng
                if (std::isalnum(c) || c == ' ')
                {
                    if (input_name_.length() < 15)
                    {
                        input_name_ += c;
                        error_text_->setString("");
                    }
                    else
                    {
                        error_text_->setString("ERROR: MAX LIMIT REACHED (15 CHARS)!");
                    }
                }
                else
                {
                    error_text_->setString("ERROR: SPECIAL CHARACTERS NOT ALLOWED!");
                }
            }
            // Cập nhật chuỗi hiển thị có kèm con trỏ "_" nhấp nháy giả lập
            input_display_text_->setString(input_name_ + "_");
            sf::FloatRect input_bounds = input_display_text_->getLocalBounds();
            input_display_text_->setPosition(
                {(Constants::WINDOW_WIDTH - input_bounds.size.x) / 2.0f, 480.0f});
        }

        // 2. Xử lý nhấn Enter hoặc Esc
        if (const auto* key_pressed = event.getIf<sf::Event::KeyPressed>())
        {
            if (key_pressed->code == sf::Keyboard::Key::Enter)
            {
                if (input_name_.length() < 3)
                {
                    error_text_->setString("ERROR: NAME TOO SHORT (MIN 3 CHARS)!");
                }
                else
                {
                    saveGameData(target_slot_, input_name_);
                    current_mode_ = Mode::SelectSlot;
                }
            }
            else if (key_pressed->code == sf::Keyboard::Key::Escape)
            {
                current_mode_ = Mode::SelectSlot;
            }
        }
        return;
    }

    // ---- MODE 2: SELECT SAVE SLOT ----
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
            if (selected_index_ == 3)
            {
                context_->state_machine->pop(); // Back
            }
            else
            {
                target_slot_ = selected_index_ + 1;
                std::string dummy_name;
                int dummy_score;

                // KIỂM TRA FILE ĐÃ TỒN TẠI HAY CHƯA
                if (Serialization::getSlotMetadata(target_slot_, dummy_name, dummy_score))
                {
                    // File đã tồn tại -> Bật cảnh báo ghi đè
                    current_mode_ = Mode::ConfirmOverwrite;
                }

                else
                {
                    // Slot trống -> Vào thẳng chế độ gõ tên
                    current_mode_ = Mode::NamingMode;
                    input_name_ = "";
                    input_display_text_->setString("_");
                    error_text_->setString("");
                }
            }
        }
        else if (key_pressed->code == sf::Keyboard::Key::Escape)
        {
            Audio().playSFX(AudioSystem::SFX::Back);
            context_->state_machine->pop();
        }
    }

    if (const auto* mouse_moved = event.getIf<sf::Event::MouseMoved>())
    {
        sf::Vector2f mouse_pos = context_->window->mapPixelToCoords(mouse_moved->position);

        for (size_t i = 0; i < save_slot_texts_.size(); ++i)
        {
            // Vì là std::unique_ptr nên phải dùng dấu ->
            if (save_slot_texts_[i]->getGlobalBounds().contains(mouse_pos))
            {
                if (selected_index_ != static_cast<int>(i))
                {
                    // Trả chữ cũ về trạng thái thường
                    Resource().setTextButtonHovered(*save_slot_texts_[selected_index_], false);
                    // Đổi chữ mới sang trạng thái hover
                    selected_index_ = static_cast<int>(i);
                    Resource().setTextButtonHovered(*save_slot_texts_[selected_index_], true);
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

            for (size_t i = 0; i < save_slot_texts_.size(); ++i)
            {
                if (save_slot_texts_[i]->getGlobalBounds().contains(mouse_pos))
                {
                    selected_index_ = static_cast<int>(i);

                    if (selected_index_ == 3)
                    {
                        context_->state_machine->pop(); // Click BACK
                    }
                    else
                    {
                        target_slot_ = selected_index_ + 1;
                        std::string dummy_name;
                        int dummy_score;

                        if (Serialization::getSlotMetadata(target_slot_, dummy_name, dummy_score))
                        {
                            current_mode_ = Mode::ConfirmOverwrite;
                        }
                        else
                        {
                            current_mode_ = Mode::NamingMode;
                            input_name_ = "";
                            input_display_text_->setString("_");
                            error_text_->setString("");
                        }
                    }
                    break;
                }
            }
        }
    }
}

// Update save state (static)
void SaveState::update(float delta_time)
{
    if (is_saved_)
    {
        save_timer_ += delta_time;
        if (save_timer_ >= 1.0f)
        {
            context_->state_machine->pop();
        }
    }
}

// Render save menu
void SaveState::render(sf::RenderWindow& window)
{
    window.clear(sf::Color(25, 25, 25));

    // Background image
    if (background_sprite_)
        window.draw(*background_sprite_);

    if (current_mode_ == Mode::ConfirmOverwrite)
    {
        // Hiển thị khung cảnh báo ghi đè
        sf::FloatRect bounds = overwrite_prompt_text_->getLocalBounds();
        overwrite_prompt_text_->setPosition(
            {(Constants::WINDOW_WIDTH - bounds.size.x) / 2.0f, 450.0f});
        Resource().drawTextWithShadow(window, *overwrite_prompt_text_);
    }

    else if (current_mode_ == Mode::NamingMode)
    {
        // Hiển thị khung nhập tên
        sf::FloatRect p_bounds = prompt_text_->getLocalBounds();
        prompt_text_->setPosition({(Constants::WINDOW_WIDTH - p_bounds.size.x) / 2.0f, 350.0f});
        Resource().drawTextWithShadow(window, *prompt_text_);
        Resource().drawTextWithShadow(window, *input_display_text_);

        if (error_text_->getString() != "")
        {
            sf::FloatRect err_bounds = error_text_->getLocalBounds();
            error_text_->setPosition(
                {(Constants::WINDOW_WIDTH - err_bounds.size.x) / 2.0f, 600.0f});
            Resource().drawTextWithShadow(window, *error_text_);
        }
    }

    else
    {
        // Chế độ chọn thường: vẽ các Slot lựa chọn
        for (const auto& slot_text : save_slot_texts_)
        {
            Resource().drawTextWithShadow(window, *slot_text);
        }
    }
}

// Move selection up
void SaveState::moveUp()
{
    Resource().setTextButtonHovered(*save_slot_texts_[selected_index_], false);
    selected_index_--;
    if (selected_index_ < 0)
        selected_index_ = static_cast<int>(save_slot_texts_.size()) - 1;
    Resource().setTextButtonHovered(*save_slot_texts_[selected_index_], true);
}

// Move selection down
void SaveState::moveDown()
{
    Resource().setTextButtonHovered(*save_slot_texts_[selected_index_], false);
    selected_index_++;
    if (selected_index_ >= static_cast<int>(save_slot_texts_.size()))
        selected_index_ = 0;
    Resource().setTextButtonHovered(*save_slot_texts_[selected_index_], true);
}

// Save game data to save file
void SaveState::saveGameData(int slot_index, const std::string& name)
{
    // Lấy màn chơi đang chạy phía bên dưới Menu Pause
    PlayingState* active_game = PlayingState::getActiveInstance();

    if (active_game)
    {
        // Ghi dữ liệu trực tiếp vào file nhị phân
        std::ofstream file("data/saves/slot_" + std::to_string(slot_index) + ".bin",
                           std::ios::binary);
        if (file.is_open())
        {
            active_game->serialize(file, name);
            file.close();
            active_game->setLoadSlot(slot_index);

            // std::cout << "Successfully saved game to Slot " << slot_index << "!" << std::endl;
            save_slot_texts_[slot_index - 1]->setString("SAVE SLOT " + std::to_string(slot_index) +
                                                        " - SAVED!");

            setupUI();
        }
        else
        {
            std::cerr << "Failed to open slot file for writing!" << std::endl;
            // save_slot_texts_[slot_index - 1]->setString("SAVE SLOT " +
            // std::to_string(slot_index) + " - FAILED!");
        }
    }

    // sf::FloatRect text_bounds = save_slot_texts_[slot_index -
    // 1]->getLocalBounds(); float x_position = (Constants::WINDOW_WIDTH -
    // text_bounds.size.x) / 2.0f; float y_position = save_slot_texts_[slot_index
    // - 1]->getPosition().y; save_slot_texts_[slot_index -
    // 1]->setPosition({x_position, y_position});
    is_saved_ = false;
    save_timer_ = 0.0f;
}
