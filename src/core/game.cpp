#include "core/game.h"
// Lưu ý: Sau này bạn cần include file màn hình đầu tiên vào đây
#include "states/home_state.h" 

Game::Game() : mWindow(sf::VideoMode({Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT}), Constants::GAME_TITLE)
{
    mWindow.setFramerateLimit(24); //24 FPS

    // TODO: Nạp màn hình đầu tiên (Home Screen) vào hệ thống khi game vừa bật lên
    mStateMachine.addState(std::make_unique<HomeState>(mStateMachine));
}

void Game::run() 
{
    while (mWindow.isOpen()) 
    {
        // 1. Áp dụng các thay đổi màn hình (push, pop, replace) nếu có
        mStateMachine.processStateChanges();

        // 2. Tính toán Delta Time (thời gian trôi qua giữa 2 khung hình)
        float dt = mClock.restart().asSeconds();

        // 3. Thực thi 3 bước cốt lõi của một game
        handleEvent();
        update(dt);
        render();
    }
}

void Game::handleEvent() 
{
    while (const std::optional<sf::Event> event = mWindow.pollEvent()) 
    {
        if (event->is<sf::Event::Closed>()) 
        {
            mWindow.close();
            return; 
        }

        if (!mStateMachine.isEmpty())
        {
            mStateMachine.getActiveState().handleEvent(*event);
        }
    }
}

void Game::update(float dt) 
{
    // Yêu cầu màn hình hiện tại cập nhật logic (di chuyển nhân vật, tính điểm, v.v.)
    mStateMachine.getActiveState().update(dt);
}

void Game::render() 
{
    // Bước 1: Xóa sạch màn hình của khung hình cũ (đổ màu đen)
    mWindow.clear(sf::Color::Black);

    // Bước 2: Truyền cửa sổ vào để màn hình hiện tại "vẽ" đồ họa lên đó
    mStateMachine.getActiveState().render(mWindow);

    // Bước 3: Đẩy toàn bộ bản vẽ hoàn chỉnh lên màn hình thực tế
    mWindow.display();
}