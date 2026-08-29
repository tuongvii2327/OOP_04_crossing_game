#include "core/game.h"
#include "utils/game_exceptions.h"
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
    catch (const GameException& e)
    {
        std::cerr << "CRITICAL EXCEPTION: " << e.what() << std::endl;
        std::cin.get();
        return -1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "UNKNOWN STD ERROR: " << e.what() << std::endl;
        std::cin.get();
        return -1;
    }
    return 0;
}
