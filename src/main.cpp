#include "core/game.h"
#include <SFML/Graphics.hpp>
#include <iostream>

int main(int argc, char *argv[])
{
    try
    {
        Game game;
        game.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Loi xay ra: " << e.what() << std::endl;
        std::cin.get();
        return -1;
    }
    return 0;
}
