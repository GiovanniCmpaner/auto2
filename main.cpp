
#include <stack>
#include <vector>
#include <random>
#include <utility>
#include <array>
#include <iostream>

#include <cstdint>
#include <cstdlib>



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

enum Direction {
    UP = 0,
    DOWN = 1,
    LEFT = 2,
    RIGHT = 3
};

auto make_maze(size_t height, size_t width)
{
    srand(1000);

    auto stack{ std::stack<std::tuple<int,int>>{} };
    auto matrix{ std::vector(height,std::vector(width,std::tuple{true,true,true,true})) };

    auto rd{ std::random_device{} };
    auto mt{ std::mt19937{rd()} };

    auto y{ 0 }, x{ 0 };

    stack.emplace(y, x);
    while (1)
    {
        auto directions{ std::vector<std::tuple<int,int>>{} };

        if (y > 0 and matrix[y - 1][x] == std::tuple{ true, true, true, true })
        {
            directions.emplace_back(-1, 0);
        }
        if (y < height - 1 and matrix[y + 1][x] == std::tuple{ true, true, true, true })
        {
            directions.emplace_back(+1, 0);
        }
        if (x > 0 and matrix[y][x - 1] == std::tuple{ true, true, true, true })
        {
            directions.emplace_back(0, -1);
        }
        if (x < width - 1 and matrix[y][x + 1] == std::tuple{ true, true, true, true })
        {
            directions.emplace_back(0, +1);
        }

        if (directions.size() > 0)
        {
            //auto dist{ std::uniform_int_distribution<int>{ 0, static_cast<int>(directions.size()) - 1} };
            //const auto choosen{ dist(mt) };
            const auto choosen{ rand() % directions.size() };
            const auto [j, i] { directions[choosen] };

            if (j == +1) {
                std::get<DOWN>(matrix[y][x]) = false;
                std::get<UP>(matrix[y + 1][x]) = false;
            }
            else if (j == -1) {
                std::get<UP>(matrix[y][x]) = false;
                std::get<DOWN>(matrix[y - 1][x]) = false;
            }
            else if (i == +1) {
                std::get<RIGHT>(matrix[y][x]) = false;
                std::get<LEFT>(matrix[y][x + 1]) = false;
            }
            else if (i == -1) {
                std::get<LEFT>(matrix[y][x]) = false;
                std::get<RIGHT>(matrix[y][x - 1]) = false;
            }

            y += j;
            x += i;

            stack.emplace(y, x);
        }
        else if (not stack.empty())
        {
            stack.pop();
            if (stack.empty())
            {
                break;
            }
            std::tie(y, x) = stack.top();
        }
        else
        {
            break;
        }
    }

    return matrix;
}

auto print_maze(const std::vector<std::vector<std::tuple<bool, bool, bool, bool>>>& matrix)
{
    std::cout << std::string(matrix.front().size() * 3 - matrix.front().size() + 1, '\xDB') << std::endl;

    for (auto y{ 0 }; y < matrix.size(); y++)
    {
        std::cout << '\xDB';

        for (auto x{ 0 }; x < matrix[y].size(); x++)
        {
            if (std::get<DOWN>(matrix[y][x]) && std::get<RIGHT>(matrix[y][x]))
            {
                std::cout << " \xDB";
            }
            else if (std::get<DOWN>(matrix[y][x]))
            {
                std::cout << "  ";
            }
            else if (std::get<RIGHT>(matrix[y][x]))
            {
                std::cout << " \xDB";
            }
            else {
                std::cout << "  ";
            }
        }

        std::cout << std::endl;

        std::cout << '\xDB';

        for (auto x{ 0 }; x < matrix[y].size(); x++)
        {
            if (std::get<DOWN>(matrix[y][x]) && std::get<RIGHT>(matrix[y][x]))
            {
                std::cout << "\xDB\xDB";
            }
            else if (std::get<DOWN>(matrix[y][x]))
            {
                std::cout << "\xDB\xDB";
            }
            else if (std::get<RIGHT>(matrix[y][x]))
            {
                std::cout << " \xDB";
            }
            else {
                std::cout << " \xDB";
            }
        }

        std::cout << std::endl;
    }
}


int main(int argc, char* args[])
{
    const auto maze{ make_maze(20,20) };
    print_maze(maze);

    //Simulation::init();

    //while (1)
    //{
    //
    //}

    //Simulation::end();
}

