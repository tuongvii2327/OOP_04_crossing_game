// include/core/state_machine.h
#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <stack>
#include <vector>

class State;

// Các hành động có thể thực hiện với State (Chuyển đổi màn hình)
enum class StateAction
{
    None,   // Không làm gì
    Push,   // Thêm một state mới lên đỉnh stack (vd: mở Pause menu)
    Pop,    // Gỡ bỏ state hiện tại trên đỉnh stack (vd: đóng Pause menu)
    Switch, // Thay thế state hiện tại bằng state mới (vd: từ Menu vào Game)
    Reset   // Xóa toàn bộ stack và thêm state mới (vd: Quay về Main Menu)
};

// Lớp quản lý vòng đời và sự chuyển đổi giữa các trạng thái (màn hình) của trò chơi
class StateMachine
{
private:
    std::vector<std::unique_ptr<State>> states; // Ngăn xếp lưu trữ các state đang hoạt động
    std::unique_ptr<State> pending_state;       // State đang chờ được thêm vào
    StateAction pending_action = StateAction::None; // Hành động chuyển đổi đang chờ xử lý

public:
    StateMachine();
    ~StateMachine();

    // Lên lịch thêm một state mới lên đỉnh
    void push(std::unique_ptr<State> state);

    // Lên lịch gỡ bỏ state trên cùng
    void pop();

    // Lên lịch thay thế state hiện tại bằng một state khác
    void switchState(std::unique_ptr<State> state);

    // Lên lịch làm sạch toàn bộ stack và chuyển sang state mới
    void resetTo(std::unique_ptr<State> state);

    // Xử lý các hành động chuyển đổi state đã được lên lịch (Gọi ở đầu mỗi frame)
    bool processTransitions();

    // Lấy state đang hoạt động (state nằm trên cùng của stack)
    State& getActiveState();
    
    // Kiểm tra xem máy trạng thái có đang rỗng hay không
    bool isEmpty() const { return states.empty(); }
    
    // Gọi phương thức render của state hiện tại
    void render(sf::RenderWindow& window);
};
