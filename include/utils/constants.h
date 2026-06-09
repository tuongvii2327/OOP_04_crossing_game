#pragma once

namespace Constants 
{
    const unsigned int WINDOW_WIDTH = 1400;
    const unsigned int WINDOW_HEIGHT = 1050;
    const char* const GAME_TITLE = "Crossy Road";

    enum class TrafficLightColor 
    {
        Red,
        Green,
        Yellow
    };

    enum class CarType 
    {
        RedCar,
        BlueCar,
        YellowCar
    };

    enum class TruckType 
    {
        RedTruck,
        BlueTruck,
        YellowTruck
    };
}