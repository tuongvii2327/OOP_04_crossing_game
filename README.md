OOP-CROSSY-ROAD
├───assets/                        <-- Tài nguyên thô của game (Tải vào RAM khi chạy)
│   ├───audio/                     <-- Âm thanh (Nhạc nền, tiếng bước nhảy, va chạm)
│   ├───fonts/                     <-- Font chữ (Dùng để hiển thị điểm số, UI Menu)
│   └───textures/                  <-- Hình ảnh (Spritesheet nhân vật, xe cộ, gạch nền)
├───bin/                           <-- Chứa sản phẩm thực thi cuối cùng (Nếu cần đóng gói)
├───build/                         <-- Thư mục sinh ra tự động khi biên dịch
│   └─── CrossyRoad.exe            <-- File chạy game và các file .dll đi kèm sau khi build
├───data/
│   └───saves/                     <-- File text/nhị phân lưu điểm cao (Highscore), dữ liệu game
├───libs/                          <-- Bộ thư viện bên thứ ba (SFML 3.1.0) để liên kết
│   ├───SFML-3.1.0-windows-gcc-14.2.0-mingw-64-bit/ <-- Dùng cho bản VS Code (MinGW)
│   └───SFML-3.1.0-windows-vc17-64-bit/             <-- Dùng khi mở bằng Visual Studio 2022 (MSVC)
├───include/                       <-- NƠI KHAI BÁO CÁC CLASS, INTERFACE (File .h)
│   ├───core/                      <-- Quản lý vòng lặp game (Game Loop) & State Machine
│   ├───entities/                  <-- Đối tượng trong game (Player, Car, Truck, Obstacle)
│   ├───states/                    <-- Các màn hình game (MainMenuState, GameState, PauseState)
│   ├───utils/                     <-- Tiện ích hệ thống (Hằng số Constants, ResourceManager)
│   └───world/                     <-- Môi trường (Quản lý các làn đường Lane, sinh bản đồ Map)
└───src/                           <-- NƠI TRIỂN KHAI LOGIC CHI TIẾT (File .cpp)
    ├───core/                      
    ├───entities/                  
    ├───states/                    
    ├───utils/                     
    ├───world/                     
    └───main.cpp                   <-- Điểm khởi đầu của chương trình (Hàm main)


variable:	snake_case
function:	camelCase
class/struct:	PascalCase
namespace:	PascalCase
const/constexpr:	UPPER_SNAKE_CASE 
file:	snake_case
enum:	PascalCase