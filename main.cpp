
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <thread>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>



#include "maze.hpp"
#include "simulation.hpp"
#include "simulation2.hpp"

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

//auto simulation{ Simulation{} };

int main(int argc, char* args[])
{
    //glm::vec3 original(2.0, 2.0, 0.0);
    //
    //glm::mat4 model = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0));
    //glm::mat4 projection = glm::ortho(0.0, 640.0, 0.0, 480.0);
    //glm::vec4 viewport(0.0, 0.0, 640.0, 480.0);
    //
    //glm::vec3 projected = glm::project(original, model, projection, viewport);
    //glm::vec3 unprojected = glm::unProject(projected, model, projection, viewport);
    //
    //std::cout << original.x << " " << original.y << " " << original.z << std::endl;
    //std::cout << projected.x << " " << projected.y << " " << projected.z << std::endl;
    //std::cout << unprojected.x << " " << unprojected.y << " " << unprojected.z << std::endl;
    //
    //return EXIT_SUCCESS;

    //simulation.init();
    //
    //while (1)
    //{
    //    std::this_thread::sleep_for(std::chrono::seconds(1));
    //}
    
    //simulation.end();
    if (Simulation2::init())
    {
    
        while (1)
        {
            if (not Simulation2::process())
            {
                break;
            }
        }
        Simulation2::end();
    }

    return EXIT_SUCCESS;
}

