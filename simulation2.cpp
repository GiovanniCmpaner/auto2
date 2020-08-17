/*This source code copyrighted by Lazy Foo' Productions (2004-2020)
and may not be redistributed without written permission.*/

//Using SDL, SDL OpenGL, GLEW, standard IO, and strings
#include <SDL.h>
#include <gl\glew.h>
#include <SDL_opengl.h>
#include <gl\glu.h>
#include <stdio.h>
#include <string>

namespace Simulation2
{
    //Screen dimension constants
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;

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

    //Shader loading utility programs
    void printProgramLog(GLuint program);
    void printShaderLog(GLuint shader);

    //The window we'll be rendering to
    SDL_Window* gWindow = NULL;

    SDL_Renderer* gRenderer = NULL;

    //OpenGL context
    SDL_GLContext gContext;

    //Render flag
    bool gRenderQuad = true;

    //Main loop flag
    bool quit = false;

    bool init()
    {
        //Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
            return false;
        }

        //Use OpenGL 3.1 core
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        //Create window
        SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT,  SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN, &gWindow,&gRenderer);
        if (gWindow == NULL)
        {
            printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
            return false;
        }
        if (gRenderer == NULL)
        {
            printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
            return false;
        }

        //Create context
        gContext = SDL_GL_CreateContext(gWindow);
        if (gContext == NULL)
        {
            printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
            return false;
        }

        //Initialize GLEW
        glewExperimental = GL_TRUE;
        GLenum glewError = glewInit();
        if (glewError != GLEW_OK)
        {
            printf("Error initializing GLEW! %s\n", glewGetErrorString(glewError));
            return false;
        }

        //Use Vsync
        if (SDL_GL_SetSwapInterval(1) < 0)
        {
            printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
            return false;
        }

        SDL_RenderClear(gRenderer);
        SDL_RenderPresent(gRenderer);

        //Main loop flag
        quit = false;

        //Enable text input
        SDL_StartTextInput();

        glClearColor(0, 0, 0, 0);
        glViewport(0, 0, 640, 480);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, 640, 480, 0, 1, -1);
        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_TEXTURE_2D);
        glLoadIdentity();

        return true;
    }

    void handleKeys(unsigned char key, int x, int y)
    {
        //Toggle quad
        if (key == 'q')
        {
            gRenderQuad = !gRenderQuad;
        }
    }

    void render()
    {
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glPushMatrix();
        {
            const auto x{ 100.0 };
            const auto y{ 100.0 };
            const auto w{ 50.0 };
            const auto h{ 50.0 };

            glTranslatef(x, y, 0);
            glRotatef(45, 0, 0, 1);
            glTranslatef(-w / 2, -h / 2, 0);
            glBegin(GL_QUADS);
            {
                glColor3f(0.0, 1.0, 0.0);
                glVertex2i(w, 0);
                glVertex2i(w, h);
                glVertex2i(0, h);
                glVertex2i(0, 0);
            }
            glEnd();
        }
        glPopMatrix();

        glFlush();
    }

    void end()
    {
        //Disable text input
        SDL_StopTextInput();

        //Destroy window	
        SDL_DestroyWindow(gWindow);
        gWindow = NULL;

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

            //Render quad
            render();

            //Update screen
            SDL_GL_SwapWindow(gWindow);
        }
        return not quit;
    }
}