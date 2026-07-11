#pragma once

#include <iostream>
#include <vector>
#include <string>

namespace Serialization
{
    // ----- GHI DỮ LIỆU NGUYÊN THỦY (float, int, bool, enum) -----
    template <typename T>
    inline void write(std::ostream &os, const T &value)
    {
        os.write(reinterpret_cast<const char *>(&value), sizeof(T));
    }

    // ----- ĐỌC DỮ LIỆU NGUYÊN THỦY -----
    template <typename T>
    inline void read(std::istream &is, T &value)
    {
        is.read(reinterpret_cast<char *>(&value), sizeof(T));
    }

    // ----- GHI CHUỖI  -----
    inline void writeString(std::ostream &os, const std::string &str)
    {
        size_t size = str.size();
        write(os, size); // Ghi kích thước chuỗi trước
        os.write(str.data(), size); // Ghi mảng ký tự sau
    }

    // ----- ĐỌC CHUỖI -----
    inline void readString(std::istream &is, std::string &str)
    {
        size_t size = 0;
        read(is, size); // Đọc kích thước chuỗi trước
        str.resize(size);
        is.read(&str[0], size); // Đọc mảng ký tự ghi đè vào chuỗi
    }
}