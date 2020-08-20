/*This source code copyrighted by Lazy Foo' Productions (2004-2020)
and may not be redistributed without written permission.*/

//Using SDL, SDL OpenGL, GLEW, standard IO, and strings
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL2_framerate.h>
#include <gl/glu.h>
#include <gl/glew.h>
#include <box2d/box2d.h>
#include <stdio.h>
#include <string>

namespace Simulation2
{
    //Screen dimension constants
    static constexpr int SCREEN_WIDTH{ 640 };
    static constexpr int SCREEN_HEIGHT{ 480 };

    //Starts up SDL, creates window, and initializes OpenGL
    bool init();

    //Initializes rendering program and clear color
    bool initGL();

    //Input handler
    void handleKeys(unsigned char key, int x, int y);

    //Per frame update
    void update();

    //Renders quad to the screen
    void render();

    //Frees media and shuts down SDL
    void end();

    //The window we'll be rendering to
    SDL_Window* gWindow{ nullptr };

    //OpenGL context
    SDL_GLContext gContext{ 0 };

    FPSmanager manager{};

    b2World world{ b2Vec2{ 0.0, 0.0 } };

    //Main loop flag
    bool quit{ false };

    bool init()
    {
        //Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO) != 0)
        {
            printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
            return false;
        }

        //Use OpenGL 3.1 core
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        //Create window
        gWindow = SDL_CreateWindow("Simulation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT,  SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
        if (gWindow == nullptr)
        {
            printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
            return false;
        }

        //Create context
        gContext = SDL_GL_CreateContext(gWindow);
        if (gContext == nullptr)
        {
            printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
            return false;
        }

        //Initialize GLEW
        glewExperimental = GL_TRUE;
        const auto glewError{ glewInit() };
        if (glewError != GLEW_OK)
        {
            printf("Error initializing GLEW! %s\n", glewGetErrorString(glewError));
            return false;
        }

        //Use Vsync
        if (SDL_GL_SetSwapInterval(1) != 0)
        {
            printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
            return false;
        }

        //Main loop flag
        quit = false;

        //Enable text input
        SDL_StartTextInput();

        // FPS
        SDL_initFramerate(&manager);
        SDL_setFramerate(&manager, 60);

        return true;
    }

    void handleKeys(unsigned char key, int x, int y)
    {

    }

    void end()
    {
        //Disable text input
        SDL_StopTextInput();

        //Destroy window	
        SDL_DestroyWindow(gWindow);
        gWindow = nullptr;

        //Quit SDL subsystems
        SDL_Quit();
    }

    auto process() -> bool
    {
        if (not quit)
        {
            //Event handler
            SDL_Event e;

            //Handle events on queue
            while (SDL_PollEvent(&e) != 0)
            {
                //User requests quit
                if (e.type == SDL_QUIT)
                {
                    quit = true;
                }
                //Handle keypress with current mouse position
                else if (e.type == SDL_TEXTINPUT)
                {
                    int x = 0, y = 0;
                    SDL_GetMouseState(&x, &y);
                    handleKeys(e.text.text[0], x, y);
                }
            }

            {
                static constexpr auto timeStep{ 1.0 / 60.0 };
                static constexpr auto velocityIterations{ 4 };
                static constexpr auto positionIterations{ 4 };

                // World step
                world.Step(timeStep, velocityIterations, positionIterations);

                // Internal draw
                draw.DrawPolygon();

                // Flush draw
                draw.Flush();
            }

            //Update screen
            SDL_GL_SwapWindow(gWindow);
            SDL_framerateDelay(&manager);
        }
        return not quit;
    }
}