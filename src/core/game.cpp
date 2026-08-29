#include "core/game.h"
#include "states/home_state.h"
#include "states/loading_state.h"
#include "utils/audio_system.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"
#include "utils/serialization.h"
#include <algorithm>
#include <fstream>
#include <iostream>

Game::Game()
{
    // Lấy thông tin độ phân giải màn hình khả dụng (Desktop Mode) của thiết bị người dùng
    const sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    const unsigned int screen_width = desktop.size.x;
    const unsigned int screen_height = desktop.size.y;

    // Kích thước chuẩn mặc định của trò chơi (1400x1050 - tỷ lệ 4:3)
    unsigned int window_width = Constants::WINDOW_WIDTH;
    unsigned int window_height = Constants::WINDOW_HEIGHT;

    // Giới hạn chiều cao cửa sổ không vượt quá 80% - 85% chiều cao màn hình khả dụng (chọn 85%)
    // để chừa khoảng trống an toàn cho thanh Titlebar và Taskbar của Windows
    const float max_ratio = 0.85f;
    const unsigned int max_safe_height = static_cast<unsigned int>(screen_height * max_ratio);

    if (window_height > max_safe_height)
    {
        window_height = max_safe_height;
        // Tính toán lại window_width giữ nguyên tỷ lệ khung hình 4:3 (1400 / 1050 = 4 / 3)
        window_width = static_cast<unsigned int>(window_height * (4.0f / 3.0f));
    }

    // Đảm bảo window_width không vượt quá max_ratio chiều rộng màn hình khả dụng
    const unsigned int max_safe_width = static_cast<unsigned int>(screen_width * max_ratio);
    if (window_width > max_safe_width)
    {
        window_width = max_safe_width;
        window_height = static_cast<unsigned int>(window_width * (3.0f / 4.0f));
    }

    // Khởi tạo cửa sổ game với kích thước đã được điều chỉnh thích ứng (Adaptive Windowing)
    window.create(sf::VideoMode({window_width, window_height}),
                  Constants::GAME_TITLE, sf::Style::Titlebar | sf::Style::Close);

    // Khóa vùng nhìn Game Logic bằng sf::View với kích thước khung nhìn chuẩn 1400x1050.
    // Mọi logic game, tọa độ vật thể, HUD và UI vẫn giữ nguyên 1400x1050, SFML tự động Scale vừa vặn cửa sổ thực tế.
    const sf::View default_view(sf::FloatRect(
        {0.f, 0.f},
        {static_cast<float>(Constants::WINDOW_WIDTH), static_cast<float>(Constants::WINDOW_HEIGHT)}
    ));
    window.setView(default_view);

    // Giới hạn FPS để đảm bảo trò chơi chạy mượt mà mà không tiêu tốn quá nhiều tài nguyên
    window.setFramerateLimit(Constants::FPS_LIMIT);

    context_.window = &window;
    context_.state_machine = &state_machine;
    context_.config = &config_;
    context_.data = &data_;

    // Khởi tạo LoadingState và render ngay Frame 0 lên cửa sổ trước khi đọc bất kỳ file nào,
    // đảm bảo màn hình Loading xuất hiện tức thì — không có màn hình trắng của Windows.
    state_machine.push(std::make_unique<LoadingState>(&context_));
    state_machine.processTransitions();
    render();

    // Đọc danh sách điểm cao (High score) đã được mã hóa ở dạng binary
    std::ifstream file("data/highscore.bin", std::ios::binary);
    if (file.is_open())
    {
        data_.high_scores.resize(5);
        file.read(reinterpret_cast<char*>(data_.high_scores.data()), 5 * sizeof(int));
        file.close();
    }

    // Đảm bảo danh sách luôn có 5 giá trị để hiển thị
    while (data_.high_scores.size() < 5)
    {
        data_.high_scores.push_back(0);
    }

    // Sắp xếp lại bảng điểm từ cao đến thấp để tiện hiển thị
    std::sort(data_.high_scores.begin(), data_.high_scores.end(), std::greater<int>());

    // Tải dữ liệu tiến trình (số lượng coin, nhân vật hiện tại) của người chơi
    loadPlayerData();
}

Game::~Game()
{
    savePlayerData();

    state_machine.resetTo(nullptr);
    state_machine.processTransitions();

    Audio().clearAll();
    Resource().clearAll();
}

void Game::run()
{
    // Đảm bảo đồng hồ được reset đúng lúc bắt đầu vòng lặp,
    // loại bỏ toàn bộ thời gian khởi tạo cửa sổ và đọc file để frame đầu tiên không bị nhảy vọt dt.
    clock.restart();

    while (window.isOpen())
    {
        if (state_machine.processTransitions())
        {
            // Reset đồng hồ khi có sự chuyển đổi state để delta time không bị tính dư từ các xử lý trước đó
            clock.restart();
        }

        // Thoát vòng lặp nếu stack state trống (Người dùng đã thoát hoàn toàn ra khỏi game)
        if (state_machine.isEmpty())
        {
            window.close();
            break;
        }

        // Khóa dt tối đa (max_dt) để tránh lag spike quá dài làm lỗi physics. 
        // Lấy 3.0f chia cho FPS_LIMIT để đảm bảo game không bị slow-motion khi FPS_LIMIT thấp.
        const float max_dt = 3.0f / static_cast<float>(Constants::FPS_LIMIT);
        float dt = std::min(clock.restart().asSeconds(), max_dt);
        handleEvent();
        update(dt);
        render();
    }
}

void Game::handleEvent()
{
    while (const std::optional<sf::Event> event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            window.close();
            return;
        }

        if (!state_machine.isEmpty())
        {
            state_machine.getActiveState().handleEvent(*event);
        }
    }
}

void Game::update(float dt)
{
    Audio().update();
    state_machine.getActiveState().update(dt);
}

void Game::render()
{
    window.clear(sf::Color::Black);
    state_machine.render(window);
    window.display();
}

void Game::loadPlayerData()
{
    std::ifstream file("data/player_data.bin", std::ios::binary);
    if (file.is_open())
    {
        Serialization::read(file, data_.total_coins);
        Serialization::read(file, data_.selected_character);

        // Đọc tuần tự trạng thái mở khóa của 3 nhân vật hiện tại
        for (int i = 0; i < 3; ++i)
        {
            bool is_unlocked;
            Serialization::read(file, is_unlocked);
            data_.unlocked_characters[i] = is_unlocked;
        }

        file.close();

        // ─── KIỂM TRA KHÓA MÃ HÓA BỊ LỆCH / DỮ LIỆU BIẾN DẠNG ───
        bool is_corrupted = false;

        // 1. Kiểm tra số Coin hợp lệ
        if (data_.total_coins < 0 || data_.total_coins > 1000000)
        {
            is_corrupted = true;
        }

        // 2. Kiểm tra chỉ số nhân vật chọn hợp lệ (chỉ từ 0 đến 2)
        if (data_.selected_character < 0 || data_.selected_character > 2)
        {
            is_corrupted = true;
        }

        // 3. Nhân vật mặc định đầu tiên (index 0) bắt buộc phải luôn được mở khóa
        if (!data_.unlocked_characters[0])
        {
            is_corrupted = true;
        }

        // ─── XỬ LÝ KHI PHÁT HIỆN LỆCH KHÓA MÃ HÓA ───
        if (is_corrupted)
        {
            // std::cout << "[Game] Key Mismatch / Corrupted Data detected! Resetting player data to default...\n";
            
            // Reset về dữ liệu mặc định cho người chơi
            data_.total_coins = 100;
            data_.selected_character = 0;
            data_.unlocked_characters = {true, false, false};

            // Lưu đè ngay lập tức bằng Secret Key mới
            savePlayerData(); 
        }
        else
        {
            if (data_.total_coins < 100)
            {
                data_.total_coins = 100;
            }
            // std::cout << "[Game] Loaded Total Coins: " << data_.total_coins << "\n";
        }
    }
    else
    {
        // Gán các giá trị mặc định cho người chơi mới nếu không tìm thấy file lưu
        data_.total_coins = 100;
        data_.selected_character = 0;
        data_.unlocked_characters = {true, false, false}; // Mặc định chỉ nhân vật đầu tiên được mở
    }
}

void Game::savePlayerData()
{
    std::ofstream file("data/player_data.bin", std::ios::binary);
    if (file.is_open())
    {
        Serialization::write(file, data_.total_coins);
        Serialization::write(file, data_.selected_character);

        // Ghi trạng thái mở khóa của 3 nhân vật
        for (int i = 0; i < 3; ++i)
        {
            bool is_unlocked = data_.unlocked_characters[i];
            Serialization::write(file, is_unlocked);
        }

        file.close();
        // std::cout << "[Game] Saved Total Coins: " << data_.total_coins << "\n";
    }
}