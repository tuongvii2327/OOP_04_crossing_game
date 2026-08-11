#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace Serialization
{
// Bảo mật: Khóa mã hóa bí mật (Secret Key) để dùng cho thuật toán XOR Cipher
inline constexpr std::string_view SECRET_KEY = "Nhom04 - OOP: Duy - Khang - Mai - Nghia - Vi";
inline constexpr size_t KEY_LEN = SECRET_KEY.size();

// Thuật toán xử lý mã hóa / giải mã XOR
inline void processXOR(char* data, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        // Thực hiện biến đổi bit XOR với Secret Key
        data[i] ^= SECRET_KEY[i % KEY_LEN];
    }
}

// Ghi dữ liệu đã mã hóa (Primitive Types) ra luồng dữ liệu
template <typename T> inline void write(std::ostream& os, const T& value)
{
    T encrypted_val = value;
    char* ptr = reinterpret_cast<char*>(&encrypted_val);

    // Mã hóa dữ liệu trước khi ghi ra đĩa
    processXOR(ptr, sizeof(T));

    os.write(ptr, sizeof(T));
}

// Đọc và giải mã dữ liệu (Primitive Types) từ luồng dữ liệu
template <typename T> inline void read(std::istream& is, T& value)
{
    is.read(reinterpret_cast<char*>(&value), sizeof(T));
    char* ptr = reinterpret_cast<char*>(&value);

    processXOR(ptr, sizeof(T));
}

// Ghi chuỗi ký tự (std::string) sau khi đã mã hóa
inline void writeString(std::ostream& os, const std::string& str)
{
    size_t size = str.size();
    write(os, size);

    if (size > 0)
    {
        std::string encrypted_str = str;
        processXOR(encrypted_str.data(), size); // Mã hóa nội dung chuỗi
        os.write(encrypted_str.data(), size);
    }
}

// Đọc và giải mã chuỗi ký tự (std::string)
inline void readString(std::istream& is, std::string& str)
{
    size_t size = 0;
    read(is, size);

    if (size > 256)
    {
        str = "";
        return;
    }

    str.resize(size);
    if (size > 0)
    {
        is.read(&str[0], size);
        processXOR(&str[0], size); // Giải mã nội dung chuỗi
    }
}

// Hàm đọc Metadata của một save slot, dữ liệu đọc ra đã được giải mã tự động
inline bool getSlotMetadata(int slot_index, std::string& out_name, int& out_score)
{
    try
    {
        std::ifstream file("data/saves/slot_" + std::to_string(slot_index) + ".bin",
                           std::ios::binary);
        if (!file.is_open())
            return false;

        readString(file, out_name);

        if (out_name.empty())
        {
            file.close();
            return false;
        }

        read(file, out_score);
        file.close();
        return true;
    }
    catch (...)
    {
        return false;
    }
}
} // namespace Serialization