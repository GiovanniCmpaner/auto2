/*This source code copyrighted by Lazy Foo' Productions (2004-2020)
and may not be redistributed without written permission.*/

//Using SDL, SDL OpenGL, GLEW, standard IO, and strings
#include <gl/glew.h>
#include <box2d/box2d.h>
#include <SDL.h>
#include <SDL2_framerate.h>
#include <cstdio>
#include <string>

#include "Camera.hpp"
#include "Draw.hpp"
#include "maze.hpp"

namespace Simulation2
{
    //Screen dimension constants
    static constexpr int SCREEN_WIDTH{ 1280 };
    static constexpr int SCREEN_HEIGHT{ 800 };

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

    b2World world{ b2Vec2{ 0.0f, -10.0f } };
    std::shared_ptr<Camera> camera{ nullptr };
    std::shared_ptr<Draw> draw{ nullptr };

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
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG | SDL_GL_CONTEXT_DEBUG_FLAG);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        

        //Create window
        gWindow = SDL_CreateWindow("Simulation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
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

        camera = std::make_shared<Camera>();
        draw = std::make_shared<Draw>(camera);

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
        //-----------------------------------------------------------------------------------------------------
        const auto tiles{ Maze::make(4,4) };
        Maze::print(tiles);
        const auto rectangles{ Maze::rectangles(tiles, 0.0f, 0.0f, 3.0f, 3.0f, 0.05f) };

        for (const auto& rect : rectangles)
        {
            b2PolygonShape polygon{};
            polygon.SetAsBox(rect.width / 2.0f, rect.height / 2.0f);

            b2BodyDef bd{};
            bd.type = b2_staticBody;
            bd.position.Set(rect.x + rect.width / 2.0f, rect.y + rect.height / 2.0f);

            const auto wall{ world.CreateBody(&bd) };
            wall->CreateFixture(&polygon, 0.0f);
        }
        //-----------------------------------------------------------------------------------------------------

        // Internal draw
        const b2Vec2 vertices[]{
            { 2.0f, 2.0f },
            { 2.0f, 4.0f },
            { 6.0f, 2.0f },
            { 2.0f, 2.0f },
        };
        b2PolygonShape chassis{};
        chassis.Set(vertices, std::size(vertices));

        b2BodyDef bd;
        bd.type = b2_dynamicBody;
        bd.position.Set(0.0f, 1.0f);
        const auto car{ world.CreateBody(&bd) };
        car->CreateFixture(&chassis, 1.0f);

        //car->ApplyForce({ 50.0f, 0.0f }, { 2.0f, 2.0f }, true);

        const b2Color color{ 1.0f, 0.0f, 1.0f };

        //-----------------------------------------------------------------------------------------------------
        while(not quit)
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
                static constexpr auto timeStep{ 1.0f / 60.0f };
                static constexpr auto velocityIterations{ 4 };
                static constexpr auto positionIterations{ 4 };

                // World step
                world.Step(timeStep, velocityIterations, positionIterations);

                glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                //----------------------------------------------------------------------------------------------
                
                for (auto body{ world.GetBodyList() }; body != nullptr; body = body->GetNext())
                {
                    const auto transform{ body->GetTransform() };
                    for (auto fixture{ body->GetFixtureList() }; fixture != nullptr; fixture = fixture->GetNext())
                    {
                        const auto polygon{ reinterpret_cast<b2PolygonShape*>(fixture->GetShape()) };
                        const auto vertexCount{ polygon->m_count };
                        b2Assert(vertexCount <= b2_maxPolygonVertices);
                        b2Vec2 vertices[b2_maxPolygonVertices];

                        for (int32 i = 0; i < vertexCount; ++i)
                        {
                            vertices[i] = b2Mul(transform, polygon->m_vertices[i]);
                        }
                        draw->drawSolidPolygon(vertices, vertexCount, color);
                    }
                }
                //----------------------------------------------------------------------------------------------
                // Flush draw
                draw->flush();
            }

            //Update screen
            SDL_GL_SwapWindow(gWindow);
            SDL_framerateDelay(&manager);
        }
        return false;
    }
}