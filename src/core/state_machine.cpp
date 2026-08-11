// src/core/state_machine.cpp
#include "core/state_machine.h"
#include "states/state.h"
#include <stdexcept>

StateMachine::StateMachine() {}

StateMachine::~StateMachine() = default;

void StateMachine::push(std::unique_ptr<State> state)
{
    pending_action = StateAction::Push;
    pending_state = std::move(state);
}

void StateMachine::pop() { pending_action = StateAction::Pop; }

void StateMachine::switchState(std::unique_ptr<State> state)
{
    pending_action = StateAction::Switch;
    pending_state = std::move(state);
}

void StateMachine::resetTo(std::unique_ptr<State> state)
{
    pending_action = StateAction::Reset;
    pending_state = std::move(state);
}

bool StateMachine::processTransitions()
{
    // Không có yêu cầu chuyển đổi nào đang chờ
    if (pending_action == StateAction::None)
        return false;

    // Nếu hành động là Pop hoặc Switch thì loại bỏ state ở đỉnh stack
    if (pending_action == StateAction::Pop || pending_action == StateAction::Switch)
    {
        if (!states.empty())
        {
            states.pop_back();
        }
    }

    // Nếu hành động là Reset thì làm sạch toàn bộ stack
    if (pending_action == StateAction::Reset)
    {
        states.clear();
    }

    // Xử lý việc thêm state mới nếu có
    if (pending_state)
    {

        // Tạm dừng state bên dưới nếu chỉ đẩy thêm state mới lên đỉnh (Push)
        if (!states.empty() && pending_action == StateAction::Push)
        {
            states.back()->pause();
        }

        states.push_back(std::move(pending_state));
        states.back()->init(); // Khởi tạo state mới
    }
    // Nếu chỉ là Pop (xóa state trên cùng) thì tiếp tục chạy state nằm ngay bên dưới
    else if (!states.empty() && pending_action == StateAction::Pop)
    {
        states.back()->resume();
    }

    // Xóa cờ trạng thái đang chờ
    pending_action = StateAction::None;
    return true;
}

State& StateMachine::getActiveState()
{
    if (states.empty())
    {
        throw std::runtime_error("No active state in StateMachine!");
    }
    return *states.back();
}

void StateMachine::render(sf::RenderWindow& window)
{
    if (states.empty())
        return;

    // Tìm vị trí state dưới cùng cần được render.
    // Nếu state hiện tại là Overlay (như Pause Menu mờ mờ), cần vẽ cả state nằm dưới nó
    int start_idx = states.size() - 1;
    while (start_idx > 0 && states[start_idx]->isOverlay())
    {
        start_idx--;
    }

    // Vẽ lần lượt từ dưới lên trên
    for (int i = start_idx; i < states.size(); ++i)
    {
        states[i]->render(window);
    }
}
