/*This source code copyrighted by Lazy Foo' Productions (2004-2020)
and may not be redistributed without written permission.*/

//Using SDL and standard IO
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL2_gfxPrimitives.h>
#include <cstdlib>
#include <iostream>
#include <string>

//Screen dimension constants
static constexpr auto screenWidth{ 640 };
static constexpr auto screenHeight{ 480 };

SDL_Window* window{ nullptr };
SDL_Renderer* renderer{ nullptr };
TTF_Font* font{ nullptr };
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
bool init()
{
    //Initialization flag
    auto success{ true };

    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cout << "SDL could not initialize! Error: " << SDL_GetError() << std::endl;
        success = false;
    }
    else
    {
        //Set texture filtering to linear
        if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
        {
            std::cout << "Warning: Linear texture filtering not enabled!" << std::endl;
        }

        //Create window
        window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
        if (window == NULL)
        {
            std::cout << "Window could not be created! Error: " << SDL_GetError() << std::endl;
            success = false;
        }
        else
        {
            //Create renderer for window
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            if (renderer == NULL)
            {
                std::cout << "Renderer could not be created! Error: " << SDL_GetError() << std::endl;
                success = false;
            }
            else
            {
                //Initialize renderer color
                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
            }
        }
    }

    if (TTF_Init() != 0)
    {
        std::cout << "TTF could not initialize! Error: " << TTF_GetError() << std::endl;
        success = false;
    }
    else
    {
        // Open font
        font = TTF_OpenFont("C:\\Windows\\Fonts\\Arial.ttf", 16);
        if (font == nullptr)
        {
            std::cerr << "Failed to open font! Error: " << TTF_GetError() << std::endl;
            return EXIT_FAILURE;
        }
    }

    return success;
}

void close()
{
    //Destroy window	
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    window = nullptr;
    renderer = nullptr;

    //Destroy font
    TTF_CloseFont(font);
    font = nullptr;

    //Quit subsystems
    SDL_Quit();
    TTF_Quit();
}

void text(int x, int y, const std::string& texto)
{
    auto rect{ SDL_Rect{x,y,0,0} };
    TTF_SizeText(font, texto.c_str(), &rect.w, &rect.h);

    if (const auto surface{ TTF_RenderText_Solid(font, texto.c_str(), { 0, 0, 0 }) }; surface == nullptr)
    {
        std::cerr << "Failed to render text! Error: " << TTF_GetError() << std::endl;
    }
    else if (const auto texture{ SDL_CreateTextureFromSurface(renderer, surface) }; texture == nullptr)
    {
        std::cerr << "Failed to create texture! Error: " << SDL_GetError() << std::endl;
    }
    else
    {
        SDL_RenderCopy(renderer, texture, nullptr, &rect);
    }
}

enum KeyPressSurfaces
{
    KEY_PRESS_DEFAULT,
    KEY_PRESS_UP,
    KEY_PRESS_DOWN,
    KEY_PRESS_LEFT,
    KEY_PRESS_RIGHT,
    KEY_PRESS_TOTAL
};
bool keyPressStates[KEY_PRESS_TOTAL]{ };

int main(int argc, char* args[])
{
    //Initialize SDL
    if (not init())
    {
        std::cout << "Failed to initialize!" << std::endl;
    }
    else
    {
        auto px{ 50.0 };
        auto py{ 50.0 };
        auto ax{ 0.0 };
        auto ay{ 0.0 };
        auto vx{ 0.0 };
        auto vy{ 0.0 };



        // Texto
        const auto texto{ "teste de texto" };


        //SDL_DestroyTexture(texture);
        //SDL_FreeSurface(surface);
        //TTF_CloseFont(font);

        //Main loop flag
        auto quit{ false };
        auto previousTicks{ uint32_t{0} };
        while (not quit)
        {
            //Event handler
            auto e{ SDL_Event{} };
            while (SDL_PollEvent(&e) != 0)
            {
                //User requests quit
                if (e.type == SDL_QUIT)
                {
                    quit = true;
                }
                else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
                {
                    if (e.key.keysym.sym == SDLK_UP)
                    {
                        keyPressStates[KEY_PRESS_UP] = (e.type == SDL_KEYDOWN);
                    }
                    else if (e.key.keysym.sym == SDLK_DOWN)
                    {
                        keyPressStates[KEY_PRESS_DOWN] = (e.type == SDL_KEYDOWN);
                    }
                    if (e.key.keysym.sym == SDLK_LEFT)
                    {
                        keyPressStates[KEY_PRESS_LEFT] = (e.type == SDL_KEYDOWN);
                    }
                    if (e.key.keysym.sym == SDLK_RIGHT)
                    {
                        keyPressStates[KEY_PRESS_RIGHT] = (e.type == SDL_KEYDOWN);
                    }
                }
            }

            //Calculate time step
            const auto currentTicks{ SDL_GetTicks() };
            if (currentTicks - previousTicks > 0) {
                const auto timeStep{ (currentTicks - previousTicks) / 1000.0 };
                previousTicks = currentTicks;


                //---------------------------------------------------------------------------------------------------
                {
                    const auto maxAcceleration{ 0.5 * timeStep };
                    const auto maxDeacceleration{ 0.5 * timeStep };

                    if (keyPressStates[KEY_PRESS_UP] || keyPressStates[KEY_PRESS_DOWN])
                    {
                        if (keyPressStates[KEY_PRESS_UP])
                        {
                            ay = -maxAcceleration;
                        }
                        if (keyPressStates[KEY_PRESS_DOWN])
                        {
                            ay = +maxAcceleration;
                        }
                    }
                    else
                    {
                        if (vy < 0.0)
                        {
                            ay = +maxDeacceleration;
                        }
                        else if (vy > 0.0)
                        {
                            ay = -maxDeacceleration;
                        }
                        else
                        {
                            ay = 0.0;
                        }
                    }

                    if (keyPressStates[KEY_PRESS_LEFT] || keyPressStates[KEY_PRESS_RIGHT])
                    {
                        if (keyPressStates[KEY_PRESS_LEFT])
                        {
                            ax = -maxAcceleration;
                        }
                        if (keyPressStates[KEY_PRESS_RIGHT])
                        {
                            ax = +maxAcceleration;
                        }
                    }
                    else
                    {
                        if (vx < 0.0)
                        {
                            ax = +maxDeacceleration;
                        }
                        else if (vx > 0.0)
                        {
                            ax = -maxDeacceleration;
                        }
                        else
                        {
                            ax = 0.0;
                        }
                    }
                }
                //---------------------------------------------------------------------------------------------------
                {
                    const auto maxVelocity{ 200.0 * timeStep };

                    if (keyPressStates[KEY_PRESS_UP] || keyPressStates[KEY_PRESS_DOWN])
                    {
                        vy += ay;
                    }
                    else if (vy < 0.0)
                    {
                        if (vy < -ay)
                        {
                            vy += ay;
                        }
                        else
                        {
                            vy = 0.0;
                        }
                    }
                    else if (vy > 0.0)
                    {
                        if (vy > -ay)
                        {
                            vy += ay;
                        }
                        else
                        {
                            vy = 0.0;
                        }
                    }

                    if (keyPressStates[KEY_PRESS_LEFT] || keyPressStates[KEY_PRESS_RIGHT])
                    {
                        vx += ax;
                    }
                    else if (vx < 0.0)
                    {
                        if (vx < -ax)
                        {
                            vx += ax;
                        }
                        else
                        {
                            vx = 0.0;
                        }
                    }
                    else if (vx > 0.0)
                    {
                        if (vx > -ax)
                        {
                            vx += ax;
                        }
                        else
                        {
                            vx = 0.0;
                        }
                    }

                    if (vy > maxVelocity)
                    {
                        vy = maxVelocity;
                    }
                    else if (vy < -maxVelocity)
                    {
                        vy = -maxVelocity;
                    }

                    if (vx > maxVelocity)
                    {
                        vx = maxVelocity;
                    }
                    else if (vx < -maxVelocity)
                    {
                        vx = -maxVelocity;
                    }
                }
                //---------------------------------------------------------------------------------------------------
                {
                    py += vy;
                    px += vx;

                    if (py > screenHeight)
                    {
                        py = screenHeight;
                        vy = 0.0;
                        ay = 0.0;
                    }
                    else if (py < 0.0)
                    {
                        py = 0.0;
                        vy = 0.0;
                        ay = 0.0;
                    }

                    if (px > screenWidth)
                    {
                        px = screenWidth;
                        vx = 0.0;
                        ax = 0.0;
                    }
                    else if (px < 0.0)
                    {
                        px = 0.0;
                        vx = 0.0;
                        ax = 0.0;
                    }
                }
                //---------------------------------------------------------------------------------------------------

                //Clear screen
                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderClear(renderer);

                lineRGBA(renderer, screenWidth / 2, screenHeight / 2 - 20, screenWidth / 2, screenHeight / 2 + 20, 0xFF, 0x00, 0x00, 0xFF);
                lineRGBA(renderer, screenWidth / 2 - 20, screenHeight / 2, screenWidth / 2 + 20, screenHeight / 2, 0xFF, 0x00, 0x00, 0xFF);
                circleRGBA(renderer, px, py, 10, 0xFF, 0x00, 0x00, 0xFF);

                lineRGBA(renderer, px, py, screenWidth / 2, screenHeight / 2, 0x00, 0x00, 0xFF, 0xFF);
                
                const auto distance{ sqrt(pow(px - screenWidth / 2, 2) + pow(py - screenHeight / 2, 2)) };

                text(0, 0, std::to_string(distance));

                //Update screen
                SDL_RenderPresent(renderer);
            }
        }
    }

    close();

    return EXIT_SUCCESS;
}