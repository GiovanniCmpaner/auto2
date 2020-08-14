
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <thread>
#include <chrono>

#include "maze.hpp"
#include "simulation.hpp"

/*
std::unique_ptr<SDL_Window, void(*)(SDL_Window*)> window2{ nullptr, SDL_DestroyWindow };

auto createWindow(const std::string& name, int width, int height) -> std::unique_ptr<SDL_Window, void(*)(SDL_Window*)>
{
    const auto window{ SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN) };
    if (window == nullptr)
    {
        std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
    }
    return { window, SDL_DestroyWindow };
}
*/



int main(int argc, char* args[])
{
    Simulation::init();

    while (1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    Simulation::end();
}

