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
                quit = true;
            }
        }

        {
            SDL_PumpEvents();
            const auto state{ SDL_GetKeyboardState(nullptr) };
            onKeyboardCallback(state);
        }

        {
            GPU_ClearColor(target, backgroundColor);

            GPU_MatrixMode(target, GPU_PROJECTION);
            GPU_LoadIdentity();
            //GPU_Ortho(-this->realHeight, +this->realHeight, +this->realWidth, -this->realWidth, 0, 1); // CENTERED
            GPU_Ortho(0, this->realHeight, this->realWidth, 0, 0, 1); // CORNER

            GPU_MatrixMode(target, GPU_MODEL);
            GPU_LoadIdentity();

            onRenderCallback(target);
        }
        
        {
            GPU_MatrixMode(target, GPU_PROJECTION);
            GPU_LoadIdentity();
            GPU_Ortho(0, Window::screenHeight, Window::screenWidth, 0, 0, 1);

            GPU_MatrixMode(target, GPU_MODEL);
            GPU_LoadIdentity();

            auto oss{ std::ostringstream{} };
            onInfosCallback(oss);
            
            FC_Draw(font, target, 5, 5, oss.str().data());
        }

        GPU_Flip(target);

        const auto currentTicks{ SDL_GetTicks() };
        targetTicks += Window::tickStep;
        if (targetTicks > currentTicks)
        {
            SDL_Delay(targetTicks - currentTicks);
        }
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