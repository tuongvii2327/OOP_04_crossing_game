#include "states/load_state.h"
#include "states/loading_state.h"
#include "states/playing_state.h"
#include "utils/audio_system.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"
#include "utils/serialization.h"
#include <fstream>

LoadState::LoadState(SharedContext* context) : State(context), selected_index_(0) {}

// Load font and initialize load screen
void LoadState::init()
{
    const sf::Texture& bg_tex = Resource().getTexture("assets/textures/ui/load.png");
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

// ---- HÀM ĐỌC NHANH THÔNG TIN FILE LƯU ----
bool getSlotMetadata(int slot_index, std::string& out_name, int& out_score)
{
    std::ifstream file("data/saves/slot_" + std::to_string(slot_index) + ".bin", std::ios::binary);
    if (!file.is_open())
        return false;

    // Chỉ cần đọc tên và điểm ở đầu file
    Serialization::readString(file, out_name);
    Serialization::read(file, out_score);
    file.close();
    return true;
}

// Setup load menu UI with save slots
void LoadState::setupUI()
{
    *title_text_ = Resource().createText("LOAD GAME", {72, sf::Color::Cyan});
    sf::FloatRect title_bounds = title_text_->getLocalBounds();
    title_text_->setPosition({Constants::WINDOW_WIDTH / 2.0f, 150.0f + title_bounds.size.y / 2.0f});

    std::vector<std::string> save_options = {"SAVE SLOT 1", "SAVE SLOT 2", "SAVE SLOT 3",
                                             "BACK TO MENU"};

    float initial_y = 350.0f;
    float spacing = 100.0f;

    save_slot_texts_.clear();
    for (size_t i = 0; i < save_options.size(); ++i)
    {
        std::string display_str;

        // Chỉ đọc dữ liệu cho 3 Slot đầu (i = 0, 1, 2)
        if (i < 3)
        {
            std::string slot_name;
            int slot_score = 0;
            if (getSlotMetadata(static_cast<int>(i) + 1, slot_name, slot_score))
            {
                display_str = "SLOT " + std::to_string(i + 1) + ": [" + slot_name +
                              "] - Score: " + std::to_string(slot_score);
            }
            else
            {
                display_str = "SLOT " + std::to_string(i + 1) + ": - EMPTY -";
            }
        }
        else
        {
            display_str = save_options[i]; // "BACK TO MENU"
        }

        sf::Text slot_text = Resource().createText(display_str, {70}, true);

        if (static_cast<int>(i) == selected_index_)
        {
            Resource().setTextButtonHovered(slot_text, true);
        }

        sf::FloatRect text_bounds = slot_text.getLocalBounds();
        float x_position = Constants::WINDOW_WIDTH / 2.0f;
        float y_position = initial_y + (static_cast<float>(i) * spacing) + text_bounds.size.y / 2.0f;

        slot_text.setPosition({x_position, y_position});
        save_slot_texts_.push_back(std::move(slot_text));
    }

    // Thêm nút BACK TO MENU ở cuối cùng
    // sf::Text back_text(font);
    // back_text.setString("BACK TO MENU");
    // back_text.setCharacterSize(42);
    // if (selected_index_ == 3)
    //     back_text.setFillColor(sf::Color::Yellow);
    // else
    //     back_text.setFillColor(sf::Color::White);

    // sf::FloatRect back_bounds = back_text.getLocalBounds();
    // back_text.setPosition({(Constants::WINDOW_WIDTH - back_bounds.size.x) / 2.0f, initial_y + 3 *
    // spacing}); save_slot_texts_.push_back(std::move(back_text));
}

// Handle slot selection and navigation
void LoadState::handleEvent(const sf::Event& event)
{
    // 1. Xử lý sự kiện bàn phím
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
        else if (key_pressed->code == sf::Keyboard::Key::Enter)
        {
            Audio().playSFX(AudioSystem::SFX::Click);
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
            Audio().playSFX(AudioSystem::SFX::Back);
            context_->state_machine->pop();
        }
    }

    // 2. Xử lý di chuột
    if (const auto* mouse_moved = event.getIf<sf::Event::MouseMoved>())
    {
        sf::Vector2f mouse_pos = context_->window->mapPixelToCoords(mouse_moved->position);

        for (size_t i = 0; i < save_slot_texts_.size(); ++i)
        {
            if (save_slot_texts_[i].getGlobalBounds().contains(mouse_pos))
            {
                if (selected_index_ != static_cast<int>(i))
                {
                    Resource().setTextButtonHovered(save_slot_texts_[selected_index_], false);
                    selected_index_ = static_cast<int>(i);
                    Resource().setTextButtonHovered(save_slot_texts_[selected_index_], true);

                    Audio().playSFX(AudioSystem::SFX::Hover);
                }
                break;
            }
        }
    }

    // 3. Xử lý click chuột
    if (const auto* mouse_button_pressed = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mouse_button_pressed->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mouse_pos = context_->window->mapPixelToCoords(mouse_button_pressed->position);

            for (size_t i = 0; i < save_slot_texts_.size(); ++i)
            {
                if (save_slot_texts_[i].getGlobalBounds().contains(mouse_pos))
                {
                    selected_index_ = static_cast<int>(i);
                    Audio().playSFX(AudioSystem::SFX::Click);
                    if (selected_index_ == static_cast<int>(save_slot_texts_.size()) - 1)
                    {
                        context_->state_machine->pop();
                    }
                    else
                    {
                        loadGameData(selected_index_ + 1);
                    }
                    break;
                }
            }
        }
    }
}

// Update load state (static)
void LoadState::update(float delta_time) {}

// Render load menu
void LoadState::render(sf::RenderWindow& window)
{
    window.clear(sf::Color(25, 25, 25));

    if (background_sprite_)
    {
        window.draw(*background_sprite_);
    }

    // if (title_text_)
    // {
    //     window.draw(*title_text_);
    // }

    for (const auto& slot_text : save_slot_texts_)
    {
        Resource().drawTextWithShadow(window, slot_text);
    }
}

// Move selection up
void LoadState::moveUp()
{
    Resource().setTextButtonHovered(save_slot_texts_[selected_index_], false);
    selected_index_--;
    if (selected_index_ < 0)
        selected_index_ = static_cast<int>(save_slot_texts_.size()) - 1;
    Resource().setTextButtonHovered(save_slot_texts_[selected_index_], true);
}

// Move selection down
void LoadState::moveDown()
{
    Resource().setTextButtonHovered(save_slot_texts_[selected_index_], false);
    selected_index_++;
    if (selected_index_ >= static_cast<int>(save_slot_texts_.size()))
        selected_index_ = 0;
    Resource().setTextButtonHovered(save_slot_texts_[selected_index_], true);
}

// Load game data from save file
void LoadState::loadGameData(int slot_index)
{
    // Kiểm tra xem file có thực sự tồn tại trước khi Load để tránh nạp file trống
    std::ifstream test_file("data/saves/slot_" + std::to_string(slot_index) + ".bin",
                            std::ios::binary);
    if (!test_file.is_open())
    {
        std::cout << "[LoadState] Slot " << slot_index << " is empty. Cannot load!" << std::endl;
        return; // File không tồn tại, chặn không cho load
    }
    test_file.close();

    // Khởi động PlayingState và nạp từ slot_index
    context_->state_machine->switchState(std::make_unique<PlayingState>(context_, slot_index));
}
