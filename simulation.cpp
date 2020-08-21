#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL2_gfxPrimitives.h>
#include <SDL2_framerate.h>
#include <box2d/box2d.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <future>
#include <functional>

#include "simulation.hpp"
#include "maze.hpp"
#include "car.hpp"

static std::vector<Maze::Rect> rectangles{};
static std::vector<Car> cars(1000);

auto Simulation::text(int x, int y, const std::string& texto) -> void
{
    //auto rect{ SDL_Rect{x,y,0,0} };
    //TTF_SizeText(font, texto.c_str(), &rect.w, &rect.h);
    //
    //if (const auto surface{ TTF_RenderText_Solid(font, texto.c_str(), { 0, 0, 0 }) }; surface == nullptr)
    //{
    //    std::cerr << "Failed to render text! Error: " << TTF_GetError() << std::endl;
    //}
    //else if (const auto texture{ SDL_CreateTextureFromSurface(renderer, surface) }; texture == nullptr)
    //{
    //    auto t{ SDL_CreateTextureFromSurface(nullptr,nullptr) };
    //    std::cerr << "Failed to create texture! Error: " << SDL_GetError() << std::endl;
    //}
    //else
    //{
    //    SDL_RenderCopy(renderer, texture, nullptr, &rect);
    //}
}

auto Simulation::move() -> void
{
    for (auto& car : cars)
    {
        car.move(0, 0, 0);
    }
}

auto Simulation::collisions() -> void
{
    for (auto& car : cars)
    {
        car.measure(reinterpret_cast<const SDL_Rect*>(rectangles.data()), rectangles.size());
    }
}

auto Simulation::draw() -> void
{
    const auto framerate{ SDL_getFramerate(&manager) };
    text(250, 0, "fps = " + std::to_string(framerate));

    SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
    for (const auto& rect : rectangles)
    {
        SDL_RenderFillRect(renderer, reinterpret_cast<const SDL_Rect*>(&rect));
    };

    for (auto& car : cars)
    {
        car.render(renderer);
    }
    const auto [front, left, right] = cars.front().distance();
    text(0, 0, "d=" + std::to_string(static_cast<int>(front)) + "," + std::to_string(static_cast<int>(left)) + "," + std::to_string(static_cast<int>(right)));

    //ang = (ang + 5) % 360;
    //
    //text(0, 0, "d = " + std::to_string(distance));

    //
    //SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 0xFF);
    //SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

auto Simulation::input() -> void
{
    auto event{ SDL_Event{} };
    while (SDL_PollEvent(&event) != 0)
    {
        if (event.type == SDL_QUIT)
        {
            quit = true;
        }
    }
}

auto Simulation::clear() -> void
{
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);
}

auto Simulation::update() -> void
{
    SDL_RenderPresent(renderer);
    SDL_framerateDelay(&manager);
}

auto Simulation::process() -> void
{
    while (not quit)
    {
        input();

        clear();

        move();

        collisions();

        draw();

        update();
    }
}

auto Simulation::initVideo() -> bool
{
    // Initialize SDL for video
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cerr << "SDL could not initialize! Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Set texture filtering to linear
    if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
    {
        std::cerr << "Warning: Linear texture filtering not enabled!" << std::endl;
        return false;
    }

    // Create window
    if (window == nullptr)
    {
        window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
        if (window == nullptr)
        {
            std::cerr << "Window could not be created! Error: " << SDL_GetError() << std::endl;
            return false;
        }
    }

    // Create renderer for window
    if (renderer == nullptr)
    {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (renderer == nullptr)
        {
            std::cerr << "Renderer could not be created! Error: " << SDL_GetError() << std::endl;
            return false;
        }
    }

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_initFramerate(&manager);
    SDL_setFramerate(&manager, 60);

    return true;
}

auto Simulation::initFont() -> bool
{
    // Initialize TTF for fonts
    if (TTF_Init() != 0)
    {
        std::cerr << "TTF could not initialize! Error: " << TTF_GetError() << std::endl;
        return false;
    }

    // Open font
    if (font == nullptr)
    {
        font = TTF_OpenFont("C:\\Windows\\Fonts\\Arial.ttf", 16);
        if (font == nullptr)
        {
            std::cerr << "Failed to open font! Error: " << TTF_GetError() << std::endl;
            return false;
        }
    }

    return true;
}

auto Simulation::initWorld() -> bool
{
    const auto gravity{ b2Vec2{0.0, 0.0} };
    world = new b2World{ gravity };

    auto bodyDef{ b2BodyDef{} };
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(0.0, 4.0);

    auto dynamicBox{ b2PolygonShape{} };
    dynamicBox.SetAsBox(1.0, 1.0);

    auto fixtureDef{ b2FixtureDef{} };
    fixtureDef.shape = &dynamicBox;
    fixtureDef.density = 1.0;
    fixtureDef.friction = 0.3;

    auto body{ world->CreateBody(&bodyDef) };
    body->CreateFixture(&fixtureDef);

    return true;
}

auto Simulation::init() -> bool
{
    const auto maze{ Maze::make(2,2) };
    Maze::print(maze);
    rectangles = Maze::rectangles(maze, 1, 1, 10, 10, 1);

    initVideo();
    initFont();
    initWorld();

    if (not task.valid())
    {
        quit = false;
        task = std::async(std::launch::async, std::bind(&Simulation::process, this));
    }

    return true;
}

auto Simulation::end() -> void
{
    if (task.valid())
    {
        quit = true;
        task.wait();
        task = {};
    }

    if (renderer != nullptr)
    {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window != nullptr)
    {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    if (font != nullptr)
    {
        TTF_CloseFont(font);
        font = nullptr;
    }

    SDL_Quit();
    TTF_Quit();
}