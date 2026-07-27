#include "states/shop_state.h"
#include "utils/audio_system.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"

ShopState::ShopState(SharedContext *context)
    : State(context), selected_index_(0), viewing_char_index_(0), char_sprite_(Resource().getDynamicObjectTexture(Constants::DynamicObjectType::People, Constants::ObjectColor::None))
{
}

void ShopState::init()
{
    const sf::Font &font = Resource().getFont("assets/fonts/VT323-Regular.ttf");
    title_text_ = std::make_unique<sf::Text>(font);
    coin_text_ = std::make_unique<sf::Text>(font);

    viewing_char_index_ = context_->data->selected_character; // Mở vào là xem con đang dùng luôn

    setupUI(font);
}

void ShopState::setupUI(const sf::Font &font)
{
    // Tiêu đề Shop
    title_text_->setString("CHARACTER SHOP");
    title_text_->setCharacterSize(72);
    title_text_->setFillColor(sf::Color::Yellow);
    sf::FloatRect title_bounds = title_text_->getLocalBounds();
    title_text_->setPosition({(Constants::WINDOW_WIDTH - title_bounds.size.x) / 2.0f, 120.0f});

    // Hiển thị số Xu hiện có
    coin_text_->setCharacterSize(50);
    coin_text_->setFillColor(sf::Color::Cyan);

    // Khởi tạo 3 dòng menu
    for (int i = 0; i < 3; ++i)
    {
        menu_texts_.push_back(std::make_unique<sf::Text>(font));
    }

    updateUI();
}

void ShopState::updateUI()
{
    // 1. Cập nhật Text Xu
    coin_text_->setString("YOUR COINS: " + std::to_string(context_->data->total_coins));
    sf::FloatRect coin_bounds = coin_text_->getLocalBounds();
    coin_text_->setPosition({(Constants::WINDOW_WIDTH - coin_bounds.size.x) / 2.0f, 220.0f});

    // 2. Logic Nút Hành Động (Mua / Trang Bị / Đang Dùng)
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

    // 3. Set text cho các Menu
    std::vector<std::string> display_strings = {
        "<  " + char_names_[viewing_char_index_] + "  >",
        action_str,
        "BACK TO MENU"};

    float initial_y = 380.0f;
    float spacing = 100.0f;

    for (size_t i = 0; i < menu_texts_.size(); ++i)
    {
        menu_texts_[i]->setString(display_strings[i]);
        menu_texts_[i]->setCharacterSize(55);

        if (static_cast<int>(i) == selected_index_)
            menu_texts_[i]->setFillColor(sf::Color::Yellow);
        else
            menu_texts_[i]->setFillColor(sf::Color::White);

        sf::FloatRect text_bounds = menu_texts_[i]->getLocalBounds();
        float x_position = (Constants::WINDOW_WIDTH - text_bounds.size.x) / 2.0f;
        float y_position = initial_y + (static_cast<float>(i) * spacing);

        menu_texts_[i]->setPosition({x_position, y_position});
    }

    Constants::ObjectColor preview_color = Constants::ObjectColor::None;
    switch (viewing_char_index_)
    {
    case 1:
        preview_color = Constants::ObjectColor::Blue;
        break;
    case 2:
        preview_color = Constants::ObjectColor::Green;
        break;
    case 3:
        preview_color = Constants::ObjectColor::Grey;
        break;
    }

    const sf::Texture &tex = Resource().getDynamicObjectTexture(Constants::DynamicObjectType::People, preview_color);
    char_sprite_.setTexture(tex, true);

    // Phóng to nhân vật lên 2.5 lần để dễ nhìn
    char_sprite_.setScale({2.5f, 2.5f});

    // Đặt vị trí vào khoảng trống giữa xu và chữ "< Tên Skin >"
    sf::FloatRect sprite_bounds = char_sprite_.getGlobalBounds();
    char_sprite_.setPosition({(Constants::WINDOW_WIDTH - sprite_bounds.size.x) / 2.0f,
                              280.0f});
}

void ShopState::handleEvent(const sf::Event &event)
{
    if (const auto *key_pressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (key_pressed->code == sf::Keyboard::Key::Up || key_pressed->code == sf::Keyboard::Key::W)
        {
            moveUp();
            Audio().playSFX(AudioSystem::SFX::Hover);
        }
        else if (key_pressed->code == sf::Keyboard::Key::Down || key_pressed->code == sf::Keyboard::Key::S)
        {
            moveDown();
            Audio().playSFX(AudioSystem::SFX::Hover);
        }
        else if (key_pressed->code == sf::Keyboard::Key::Left || key_pressed->code == sf::Keyboard::Key::A)
        {
            if (selected_index_ == 0)
            { // Đang đứng ở dòng Chọn Skin
                prevCharacter();
                Audio().playSFX(AudioSystem::SFX::Hover);
            }
        }
        else if (key_pressed->code == sf::Keyboard::Key::Right || key_pressed->code == sf::Keyboard::Key::D)
        {
            if (selected_index_ == 0)
            {
                nextCharacter();
                Audio().playSFX(AudioSystem::SFX::Hover);
            }
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
}

void ShopState::update(float delta_time) {}

void ShopState::render(sf::RenderWindow &window)
{
    window.clear(sf::Color(30, 30, 45)); // Màu nền tối cho shop

    if (title_text_)
        window.draw(*title_text_);
    if (coin_text_)
        window.draw(*coin_text_);

    window.draw(char_sprite_); // <--- THÊM DÒNG NÀY ĐỂ VẼ ẢNH SKIN

    for (const auto &text : menu_texts_)
    {
        if (text)
            window.draw(*text);
    }
}

void ShopState::moveUp()
{
    selected_index_--;
    if (selected_index_ < 0)
        selected_index_ = static_cast<int>(menu_texts_.size()) - 1;
    updateUI();
}

void ShopState::moveDown()
{
    selected_index_++;
    if (selected_index_ >= static_cast<int>(menu_texts_.size()))
        selected_index_ = 0;
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
    if (selected_index_ == 2) // Nút Back
    {
        Audio().playSFX(AudioSystem::SFX::Back);
        context_->state_machine->pop();
    }
    else if (selected_index_ == 1) // Nút Hành Động
    {
        bool is_unlocked = context_->data->unlocked_characters[viewing_char_index_];

        if (!is_unlocked) // Xử lý Mua
        {
            int price = char_prices_[viewing_char_index_];
            if (context_->data->total_coins >= price)
            {
                context_->data->total_coins -= price;
                context_->data->unlocked_characters[viewing_char_index_] = true;
                context_->data->selected_character = viewing_char_index_; // Mua xong thì tự động trang bị
                Audio().playSFX(AudioSystem::SFX::Click);                 // Tiếng mua thành công
            }
            else
            {
                Audio().playSFX(AudioSystem::SFX::Crash); // Tiếng báo lỗi (không đủ tiền)
            }
        }
        else // Xử lý Trang Bị
        {
            if (context_->data->selected_character != viewing_char_index_)
            {
                context_->data->selected_character = viewing_char_index_;
                Audio().playSFX(AudioSystem::SFX::Click);
            }
        }
        updateUI(); // Render lại chữ (từ BUY -> EQUIPPED, cập nhật trừ tiền...)
    }
}