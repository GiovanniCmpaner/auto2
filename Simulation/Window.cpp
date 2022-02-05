#include "Window.hpp"

auto Window::init(float realWidth, float realHeight) -> void
{
    this->realWidth = realWidth;
    this->realHeight = realHeight;

    GPU_SetDebugLevel(GPU_DEBUG_LEVEL_MAX);
    GPU_SetRequiredFeatures(GPU_FEATURE_BASIC_SHADERS);
    this->target = GPU_InitRenderer(GPU_RENDERER_OPENGL_3, Window::screenWidth, Window::screenHeight, GPU_DEFAULT_INIT_FLAGS);
    //SDL_AddEventWatch(resizingEventWatcher, nullptr);

    this->font = FC_CreateFont();
    FC_LoadFont(this->font, "C:/Windows/Fonts/Arial.ttf", Window::screenHeight / 40, fontColor, TTF_STYLE_NORMAL);
}

auto Window::process() -> void
{
    auto targetTicks{ SDL_GetTicks() };

    while (not quit)
    {
        SDL_Event e{};
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                this->quit = true;
            }
        }

        {
            SDL_PumpEvents();
            auto numKeys{ 0 };
            const auto keyboardState{ SDL_GetKeyboardState(&numKeys) };
            this->onKeyboardCallback(keyboardState);
            if (keyboardState[SDL_SCANCODE_RETURN])
            {
                this->xOffset = 0;
                this->yOffset = 0;
            }

            auto x{ -1 }, y{ -1 };
            const auto mouseState{ SDL_GetMouseState(&x, &y) };

            { // drag
                if (not this->dragging)
                {
                    if (mouseState & SDL_BUTTON_LMASK)
                    {
                        this->dragging = true;
                        this->xPos = x;
                        this->yPos = y;
                    }
                }
                else if (x > -1 and y > -1)
                {
                    if (not (mouseState & SDL_BUTTON_LMASK))
                    {
                        this->dragging = false;
                    }

                    this->xOffset += (this->xPos - x) / 200.0f;
                    this->yOffset += (this->yPos - y) / 200.0f;

                    this->xPos = x;
                    this->yPos = y;
                }
            }
        }

        {
            GPU_ClearColor(target, backgroundColor);

            GPU_MatrixMode(target, GPU_PROJECTION);
            GPU_LoadIdentity();
            //GPU_Ortho(-this->realHeight, +this->realHeight, +this->realWidth, -this->realWidth, 0, 1); // CENTERED
            GPU_Ortho(this->xOffset, this->xOffset + this->realHeight, this->yOffset + this->realWidth, this->yOffset, 0, 1); // CORNER

            GPU_MatrixMode(target, GPU_MODEL);
            GPU_LoadIdentity();

            this->onRenderCallback(target);
        }
        
        {
            GPU_MatrixMode(target, GPU_PROJECTION);
            GPU_LoadIdentity();
            GPU_Ortho(this->xOffset, this->xOffset + Window::screenHeight, Window::screenWidth, 0, 0, 1);

            GPU_MatrixMode(target, GPU_MODEL);
            GPU_LoadIdentity();

            auto oss{ std::ostringstream{} };
            this->onInfosCallback(oss);
            
            FC_Draw(font, target, 5, 5, oss.str().data());
        }

        GPU_Flip(target);

        //const auto currentTicks{ SDL_GetTicks() };
        //targetTicks += Window::tickStep;
        //if (targetTicks > currentTicks)
        //{
        //    SDL_Delay(targetTicks - currentTicks);
        //}

        SDL_Delay(Window::tickStep);
        this->time += Window::tickStep;
    }

    GPU_Quit();
}

auto Window::onKeyboard(std::function<void(const uint8_t*)> callback) -> void
{
    this->onKeyboardCallback = callback;
}

auto Window::onRender(std::function<void(GPU_Target*)> callback) -> void
{
    this->onRenderCallback = callback;
}

auto Window::onInfos(std::function<void(std::ostringstream&)> callback) -> void
{
    this->onInfosCallback = callback;
}

auto Window::now() -> unsigned long long
{
    return this->time;
}