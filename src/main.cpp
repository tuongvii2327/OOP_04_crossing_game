#include "core/game.h"
#include <SFML/Graphics.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
    try
    {
        // Khởi tạo đối tượng Game và bắt đầu vòng lặp chính
        Game game;
        game.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Loi xay ra: " << e.what() << std::endl;
        std::cin.get();
        return -1;
    }
    return 0;
}
