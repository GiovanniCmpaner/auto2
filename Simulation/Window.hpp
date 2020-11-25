#pragma once

#include <iostream>
#include <sstream>
#include <functional>

#include <box2d/box2d.h>
#include <SDL_gpu.h>
#include <SDL_FontCache.h>

class Window
{
public:
    auto init(float realWidth, float realHeight) -> void;
    auto process() -> void;

    auto onKeyboard(std::function<void(const uint8_t*)> callback) -> void;
    auto onRender(std::function<void(GPU_Target*)> callback) -> void;
    auto onInfos(std::function<void(std::ostringstream&)> callback) -> void;

    auto now() -> unsigned long long;

    static constexpr int screenWidth{ 1000 };
    static constexpr int screenHeight{ 1000 };

    static constexpr int fps{ 30 };
    static constexpr float timeStep{ 1.0f / fps };
    static constexpr float tickStep{ 1000 / fps };

    static constexpr SDL_Color backgroundColor{ 0, 0, 0, 255 };
    static constexpr SDL_Color fontColor{ 0, 255, 0, 255 };
    static constexpr SDL_Color sensorColor{ 0,0,255,255 };
    static constexpr SDL_Color solidBorderColor{ 255, 0, 255, 255 };
    static constexpr SDL_Color solidFillColor{ 255, 0, 255, 64 };

private:
    GPU_Target* target{ nullptr };
    FC_Font* font{ nullptr };
    bool quit{ false };
    
    float realWidth{ NAN };
    float realHeight{ NAN };
    unsigned long long time{ 0 };

    std::function<void(const uint8_t*)> onKeyboardCallback;
    std::function<void(GPU_Target*)> onRenderCallback;
    std::function<void(std::ostringstream&)> onInfosCallback;
};