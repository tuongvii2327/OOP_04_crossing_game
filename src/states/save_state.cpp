// src/states/save_state.cpp
#include "states/save_state.h"
#include "utils/audio_system.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"

SaveState::SaveState(SharedContext *context)
    : State(context), selected_index_(0), is_saved_(false), save_timer_(0.0f)
{
}

// Load font and initialize save screen
void SaveState::init()
{
    const sf::Font &font = Resource().getFont("assets/fonts/VT323-Regular.ttf");
    title_text_ = std::make_unique<sf::Text>(font);

    // Khởi tạo các Text hiển thị cho chế độ gõ tên
    prompt_text_ = std::make_unique<sf::Text>(font);
    input_display_text_ = std::make_unique<sf::Text>(font);
    error_text_ = std::make_unique<sf::Text>(font);

    setupUI(font);
}

// Setup save menu UI with save slots
void SaveState::setupUI(const sf::Font &font)
{
    // 1. UI BAN ĐẦU (CHỌN SLOT)
    title_text_->setString("SAVE GAME");
    title_text_->setCharacterSize(72);
    title_text_->setFillColor(sf::Color::Green);

    sf::FloatRect title_bounds = title_text_->getLocalBounds();
    float title_x = (Constants::WINDOW_WIDTH - title_bounds.size.x) / 2.0f;
    title_text_->setPosition({title_x, 150.0f});

    std::vector<std::string> save_options = {
        "SAVE SLOT 1",
        "SAVE SLOT 2",
        "SAVE SLOT 3",
        "BACK"};

    float initial_y = 350.0f;
    float spacing = 80.0f;

    for (size_t i = 0; i < save_options.size(); ++i)
    {
        auto slot_text = std::make_unique<sf::Text>(font);
        slot_text->setString(save_options[i]);
        slot_text->setCharacterSize(48);

        if (static_cast<int>(i) == selected_index_)
        {
            slot_text->setFillColor(sf::Color::Yellow);
        }
        else
        {
            slot_text->setFillColor(sf::Color::White);
        }

        sf::FloatRect text_bounds = slot_text->getLocalBounds();
        float x_position = (Constants::WINDOW_WIDTH - text_bounds.size.x) / 2.0f;
        float y_position = initial_y + (static_cast<float>(i) * spacing);

        slot_text->setPosition({x_position, y_position});
        save_slot_texts_.push_back(std::move(slot_text));
    }

    // 2. UI GÕ TÊN
    prompt_text_->setString("ENTER YOU NAME (3 - 15 CHARS):");
    prompt_text_->setCharacterSize(48);
    prompt_text_->setFillColor(sf::Color::Cyan);
    sf::FloatRect p_bounds = prompt_text_->getLocalBounds();
    prompt_text_->setPosition({(Constants::WINDOW_WIDTH - p_bounds.size.x) / 2.0f, 320.0f});

    input_display_text_->setString("_");
    input_display_text_->setCharacterSize(60);
    input_display_text_->setFillColor(sf::Color::Yellow);
    input_display_text_->setPosition({100.f, 420.0f});

    error_text_->setString("");
    error_text_->setCharacterSize(36);
    error_text_->setFillColor(sf::Color::Red);
}

// Handle slot selection and navigation
void SaveState::handleEvent(const sf::Event &event)
{
    // ---- MODE 1: INPUTTING NAME ----
    if (is_naming_mode_)
    {
        // 1. Xử lý gõ phím ký tự Unicode
        if (const auto *text_entered = event.getIf<sf::Event::TextEntered>())
        {
            char32_t unicode = text_entered->unicode;

            if (unicode == 8) // BACKSPACE
            {
                if (!input_name_.empty())
                    input_name_.pop_back();

                error_text_->setString("");
            }

            else if (unicode >= 32 && unicode < 127) // Các ký tự in ra được
            {
                char c = static_cast<char>(unicode);

                // RÀNG BUỘC KÝ TỰ: Chỉ chấp nhận chữ cái, số và khoảng trắng
                if (std::isalnum(c) || c == ' ')
                {
                    if (input_name_.length() < 15) // MAX 15 VALID CHARACTERS
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
            input_display_text_->setPosition({(Constants::WINDOW_WIDTH - input_bounds.size.x) / 2.0f, 420.0f});
        }

        // 2. Xử lý nhấn Enter hoặc Esc
        if (const auto *key_pressed = event.getIf<sf::Event::KeyPressed>())
        {
            if (key_pressed->code == sf::Keyboard::Key::Enter)
            {
                // MIN 3 VALID CHARACTERS
                if (input_name_.length() < 3)
                {
                    error_text_->setString("ERROR: NAME TOO SHORT (MIN 3 CHARS)!");
                }
                else
                {
                    // Đạt mọi điều kiện -> Tiến hành lưu game
                    saveGameData(target_slot_, input_name_);
                    is_naming_mode_ = false; // Thoát chế độ gõ tên
                }
            }
            else if (key_pressed->code == sf::Keyboard::Key::Escape)
            {
                is_naming_mode_ = false; // Huỷ bỏ, quay về menu chọn Slot
            }
        }
        return;
    }

    // ---- MODE 2: SELECT SAVE SLOT ----
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
            if (selected_index_ == static_cast<int>(save_slot_texts_.size()) - 1)
            {
                context_->state_machine->pop(); // Back
            }
            else
            {
                // Thay vì lưu ngay, chuyển sang chế độ gõ tên
                is_naming_mode_ = true;
                target_slot_ = selected_index_ + 1;
                input_name_ = "";
                input_display_text_->setString("_");
                error_text_->setString("");
            }
        }
        else if (key_pressed->code == sf::Keyboard::Key::Escape)
        {
            Audio().playSFX(AudioSystem::SFX::Back);
            context_->state_machine->pop();
        }
    }

    if (const auto *mouse_moved = event.getIf<sf::Event::MouseMoved>())
    {
        sf::Vector2f mouse_pos(static_cast<float>(mouse_moved->position.x), static_cast<float>(mouse_moved->position.y));

        for (size_t i = 0; i < save_slot_texts_.size(); ++i)
        {
            // Vì là std::unique_ptr nên phải dùng dấu ->
            if (save_slot_texts_[i]->getGlobalBounds().contains(mouse_pos))
            {
                if (selected_index_ != static_cast<int>(i))
                {
                    // Trả chữ cũ về màu trắng
                    save_slot_texts_[selected_index_]->setFillColor(sf::Color::White);
                    // Đổi chữ mới thành màu vàng
                    selected_index_ = static_cast<int>(i);
                    save_slot_texts_[selected_index_]->setFillColor(sf::Color::Yellow);
                }
                break;
            }
        }
    }

    if (const auto *mouse_button_pressed = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mouse_button_pressed->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mouse_pos(static_cast<float>(mouse_button_pressed->position.x), static_cast<float>(mouse_button_pressed->position.y));

            for (size_t i = 0; i < save_slot_texts_.size(); ++i)
            {
                if (save_slot_texts_[i]->getGlobalBounds().contains(mouse_pos))
                {
                    selected_index_ = static_cast<int>(i);

                    if (selected_index_ == static_cast<int>(save_slot_texts_.size()) - 1)
                    {
                        context_->state_machine->pop(); // Click BACK
                    }
                    else
                    {
                        // Thay vì lưu ngay, ta chuyển sang chế độ gõ tên
                        is_naming_mode_ = true;
                        target_slot_ = selected_index_ + 1;
                        input_name_ = "";
                        input_display_text_->setString("_");
                        error_text_->setString("");
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
void SaveState::render(sf::RenderWindow &window)
{
    window.clear(sf::Color(25, 25, 25));

    // Draw title
    window.draw(*title_text_);

    if (is_naming_mode_)
    {
        // Đang gõ tên: vẽ hướng dẫn gõ, tên đang gõ, và lỗi (nếu có)
        window.draw(*prompt_text_);
        window.draw(*input_display_text_);

        if (error_text_->getString() != "")
        {
            sf::FloatRect err_bounds = error_text_->getLocalBounds();
            error_text_->setPosition({(Constants::WINDOW_WIDTH - err_bounds.size.x) / 2.0f, 520.0f});
            window.draw(*error_text_);
        }
    }
    else
    {
        // Chế độ chọn thường: vẽ các Slot lựa chọn
        for (const auto &slot_text : save_slot_texts_)
        {
            window.draw(*slot_text);
        }
    }
}

// Move selection up
void SaveState::moveUp()
{
    save_slot_texts_[selected_index_]->setFillColor(sf::Color::White);
    selected_index_--;
    if (selected_index_ < 0)
        selected_index_ = static_cast<int>(save_slot_texts_.size()) - 1;
    save_slot_texts_[selected_index_]->setFillColor(sf::Color::Yellow);
}

// Move selection down
void SaveState::moveDown()
{
    save_slot_texts_[selected_index_]->setFillColor(sf::Color::White);
    selected_index_++;
    if (selected_index_ >= static_cast<int>(save_slot_texts_.size()))
        selected_index_ = 0;
    save_slot_texts_[selected_index_]->setFillColor(sf::Color::Yellow);
}

// Save game data to save file
void SaveState::saveGameData(int slot_index, const std::string &name)
{
    // Lấy màn chơi đang chạy phía bên dưới Menu Pause
    PlayingState *active_game = PlayingState::getActiveInstance();

    if (active_game)
    {
        // Ghi dữ liệu trực tiếp vào file nhị phân
        std::ofstream file("data/saves/slot_" + std::to_string(slot_index) + ".bin", std::ios::binary);
        if (file.is_open())
        {
            active_game->serialize(file, name);
            file.close();

            std::cout << "Successfully saved game to Slot " << slot_index << "!" << std::endl;
            save_slot_texts_[slot_index - 1]->setString("SAVE SLOT " + std::to_string(slot_index) + " - SAVED!");
        }
        else
        {
            std::cerr << "Failed to open slot file for writing!" << std::endl;
            save_slot_texts_[slot_index - 1]->setString("SAVE SLOT " + std::to_string(slot_index) + " - FAILED!");
        }
    }

    sf::FloatRect text_bounds = save_slot_texts_[slot_index - 1]->getLocalBounds();
    float x_position = (Constants::WINDOW_WIDTH - text_bounds.size.x) / 2.0f;
    float y_position = save_slot_texts_[slot_index - 1]->getPosition().y;
    save_slot_texts_[slot_index - 1]->setPosition({x_position, y_position});
    is_saved_ = false;
    save_timer_ = 0.0f;
}
