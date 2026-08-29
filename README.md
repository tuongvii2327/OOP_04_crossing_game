# CROSSY ROAD (OOP Project - HCMUS)

Dự án game **Crossy Road** được xây dựng bằng ngôn ngữ **C++17** và thư viện đồ họa **SFML 3.1.0**, áp dụng các nguyên lý Lập trình hướng đối tượng (OOP) và Design Patterns.

---

## 🛠️ YÊU CẦU HỆ THỐNG
- **Hệ điều hành:** Windows 10/11 (64-bit).
- **Trình biên dịch / IDE:**
  - **Visual Studio 2022** (Đã cài gói *Desktop development with C++* và *C++ CMake tools for Windows*).
  - Hoặc **VS Code** kèm bộ biên dịch **MinGW-w64 (GCC 14.2.0 trở lên)** và **CMake (>= 3.15)**.
- **Thư viện SFML 3.1.0:** Đã được tích hợp sẵn trong thư mục `libs/`, không cần cài đặt thêm thủ công.

---

## 🚀 HƯỚNG DẪN BUILD & CHẠY GAME

### Cách 1: Sử dụng Visual Studio 2022 (Khuyên dùng)

1. **Mở dự án:**
   - Khởi động **Visual Studio 2022**.
   - Chọn **Open a local folder** (Mở thư mục cục bộ).
   - Trỏ đến thư mục chứa mã nguồn: `oop-crossy-road`.

2. **Chờ nạp CMake:**
   - Visual Studio sẽ tự động đọc `CMakeLists.txt` và `CMakeSettings.json`.
   - Đợi thanh tiến trình ở góc dưới báo `CMake generation finished`.

3. **Chọn cấu hình và chạy:**
   - Trên thanh công cụ trên cùng, chọn cấu hình: **`x64-Debug`** hoặc **`x64-Release`**.
   - Chọn mục khởi động (Startup Item): **`CrossyRoad.exe`**.
   - Nhấn **F5** (hoặc bấm nút **▶ Play**) để biên dịch và chạy game.

> *(Hệ thống build sẽ tự động sao chép thư mục `assets/`, `data/` và các file DLL của SFML vào thư mục thực thi tương ứng).*

---

### Cách 2: Sử dụng dòng lệnh (CMake CLI / MinGW)

1. Mở Terminal / PowerShell tại thư mục gốc của dự án.
2. Tạo thư mục build và cấu hình:
   ```bash
   cmake -B build -G "MinGW Makefiles"
   ```
3. Tiến hành biên dịch:
   ```bash
   cmake --build build
   ```
4. Chạy game:
   ```bash
   ./build/CrossyRoad.exe
   ```

---