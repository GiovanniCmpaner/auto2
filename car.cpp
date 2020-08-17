#include <cmath>
#include <algorithm>
#include <SDL2_gfxPrimitives.h>

#include "car.hpp"

namespace Mine {

    /* Use the Cohen-Sutherland algorithm for line clipping */
    #define CODE_BOTTOM 1
    #define CODE_TOP    2
    #define CODE_LEFT   4
    #define CODE_RIGHT  8

    static int ComputeOutCode(const SDL_Rect* rect, int x, int y)
    {
        int code = 0;
        if (y < rect->y) {
            code |= CODE_TOP;
        }
        else if (y >= rect->y + rect->h) {
            code |= CODE_BOTTOM;
        }
        if (x < rect->x) {
            code |= CODE_LEFT;
        }
        else if (x >= rect->x + rect->w) {
            code |= CODE_RIGHT;
        }
        return code;
    }

    SDL_bool SDL_IntersectRectAndLine(const SDL_Rect* rect, int* X1, int* Y1, int* X2,int* Y2)
    {
        int x = 0;
        int y = 0;
        int x1, y1;
        int x2, y2;
        int rectx1;
        int recty1;
        int rectx2;
        int recty2;
        int outcode1, outcode2;

        if (!rect) {
            SDL_InvalidParamError("rect");
            return SDL_FALSE;
        }

        if (!X1) {
            SDL_InvalidParamError("X1");
            return SDL_FALSE;
        }

        if (!Y1) {
            SDL_InvalidParamError("Y1");
            return SDL_FALSE;
        }

        if (!X2) {
            SDL_InvalidParamError("X2");
            return SDL_FALSE;
        }

        if (!Y2) {
            SDL_InvalidParamError("Y2");
            return SDL_FALSE;
        }

        /* Special case for empty rect */
        if (SDL_RectEmpty(rect)) {
            return SDL_FALSE;
        }

        x1 = *X1;
        y1 = *Y1;
        x2 = *X2;
        y2 = *Y2;
        rectx1 = rect->x;
        recty1 = rect->y;
        rectx2 = rect->x + rect->w - 1;
        recty2 = rect->y + rect->h - 1;

        /* Check to see if entire line is inside rect */
        if (x1 >= rectx1 && x1 <= rectx2 && x2 >= rectx1 && x2 <= rectx2 &&
            y1 >= recty1 && y1 <= recty2 && y2 >= recty1 && y2 <= recty2) {
            return SDL_TRUE;
        }

        /* Check to see if entire line is to one side of rect */
        if ((x1 < rectx1 && x2 < rectx1) || (x1 > rectx2 && x2 > rectx2) ||
            (y1 < recty1 && y2 < recty1) || (y1 > recty2 && y2 > recty2)) {
            return SDL_FALSE;
        }

        if (y1 == y2) {
            /* Horizontal line, easy to clip */
            if (x1 < rectx1) {
                *X1 = rectx1;
            }
            else if (x1 > rectx2) {
                *X1 = rectx2;
            }
            if (x2 < rectx1) {
                *X2 = rectx1;
            }
            else if (x2 > rectx2) {
                *X2 = rectx2;
            }
            return SDL_TRUE;
        }

        if (x1 == x2) {
            /* Vertical line, easy to clip */
            if (y1 < recty1) {
                *Y1 = recty1;
            }
            else if (y1 > recty2) {
                *Y1 = recty2;
            }
            if (y2 < recty1) {
                *Y2 = recty1;
            }
            else if (y2 > recty2) {
                *Y2 = recty2;
            }
            return SDL_TRUE;
        }

        /* More complicated Cohen-Sutherland algorithm */
        outcode1 = ComputeOutCode(rect, x1, y1);
        outcode2 = ComputeOutCode(rect, x2, y2);
        while (outcode1 || outcode2) {
            if (outcode1 & outcode2) {
                return SDL_FALSE;
            }

            if (outcode1) {
                if (outcode1 & CODE_TOP) {
                    y = recty1;
                    x = x1 + ((x2 - x1) * (y - y1)) / (y2 - y1);
                }
                else if (outcode1 & CODE_BOTTOM) {
                    y = recty2;
                    x = x1 + ((x2 - x1) * (y - y1)) / (y2 - y1);
                }
                else if (outcode1 & CODE_LEFT) {
                    x = rectx1;
                    y = y1 + ((y2 - y1) * (x - x1)) / (x2 - x1);
                }
                else if (outcode1 & CODE_RIGHT) {
                    x = rectx2;
                    y = y1 + ((y2 - y1) * (x - x1)) / (x2 - x1);
                }
                x1 = x;
                y1 = y;
                outcode1 = ComputeOutCode(rect, x, y);
            }
            else {
                if (outcode2 & CODE_TOP) {
                    y = recty1;
                    x = x1 + ((x2 - x1) * (y - y1)) / (y2 - y1);
                }
                else if (outcode2 & CODE_BOTTOM) {
                    y = recty2;
                    x = x1 + ((x2 - x1) * (y - y1)) / (y2 - y1);
                }
                else if (outcode2 & CODE_LEFT) {
                    /* If this assertion ever fires, here's the static analysis that warned about it:
                       http://buildbot.libsdl.org/sdl-static-analysis/sdl-macosx-static-analysis/sdl-macosx-static-analysis-1101/report-b0d01a.html#EndPath */
                    SDL_assert(x2 != x1);  /* if equal: division by zero. */
                    x = rectx1;
                    y = y1 + ((y2 - y1) * (x - x1)) / (x2 - x1);
                }
                else if (outcode2 & CODE_RIGHT) {
                    /* If this assertion ever fires, here's the static analysis that warned about it:
                       http://buildbot.libsdl.org/sdl-static-analysis/sdl-macosx-static-analysis/sdl-macosx-static-analysis-1101/report-39b114.html#EndPath */
                    SDL_assert(x2 != x1);  /* if equal: division by zero. */
                    x = rectx2;
                    y = y1 + ((y2 - y1) * (x - x1)) / (x2 - x1);
                }
                x2 = x;
                y2 = y;
                outcode2 = ComputeOutCode(rect, x, y);
            }
        }
        *X1 = x1;
        *Y1 = y1;
        *X2 = x2;
        *Y2 = y2;
        return SDL_TRUE;
    }

}

static constexpr auto radians(double angle) -> double 
{
    return ( angle * M_PI / 180.0 );
}

auto Car::init() -> void
{

}

auto Car::move(int horizontal, int vertical, int angle) -> void
{
    front = 9999.0;
    left = 9999.0;
    right = 9999.0;

    this->angle = (this->angle + 1 % 360);
}

auto Car::measure(const SDL_Rect* rects, size_t count) -> void
{
    {
        const auto radians{ (angle + 0) * M_PI / 180.0 };

        auto rx1{ static_cast<int>(rect.x + (rect.w / 2) * std::cos(radians)) };
        auto rx2{ static_cast<int>(rx1 + front * std::cos(radians)) };

        auto ry1{ static_cast<int>(rect.y + (rect.w / 2) * std::sin(radians)) };
        auto ry2{ static_cast<int>(ry1 + front * std::sin(radians)) };

        for (auto n{ 0 }; n < count; n++)
        {
            auto x1{ rx1 }, y1{ ry1 }, x2{ rx2 }, y2{ ry2 };
            if (SDL_IntersectRectAndLine(rects + n, &x1, &y1, &x2, &y2))
            {
                rx2 = x1;
                ry2 = y1;
            }
        }

        front = std::sqrt(std::pow(rx2 - rx1, 2) + std::pow(ry2 - ry1, 2));
    }
    {
        const auto radians{ (angle + 90) * M_PI / 180.0 };

        auto rx1{ static_cast<int>(rect.x + (rect.w / 2) * std::cos(radians)) };
        auto rx2{ static_cast<int>(rx1 + left * std::cos(radians)) };

        auto ry1{ static_cast<int>(rect.y + (rect.w / 2) * std::sin(radians)) };
        auto ry2{ static_cast<int>(ry1 + left * std::sin(radians)) };

        for (auto n{ 0 }; n < count; n++)
        {
            auto x1{ rx1 }, y1{ ry1 }, x2{ rx2 }, y2{ ry2 };
            if (SDL_IntersectRectAndLine(rects + n, &x1, &y1, &x2, &y2))
            {
                rx2 = x1;
                ry2 = y1;
            }
        }

        left = std::sqrt(std::pow(rx2 - rx1, 2) + std::pow(ry2 - ry1, 2));
    }
    {
        const auto radians{ (angle - 90) * M_PI / 180.0 };

        auto rx1{ static_cast<int>(rect.x + (rect.w / 2) * std::cos(radians)) };
        auto rx2{ static_cast<int>(rx1 + right * std::cos(radians)) };

        auto ry1{ static_cast<int>(rect.y + (rect.w / 2) * std::sin(radians)) };
        auto ry2{ static_cast<int>(ry1 + right * std::sin(radians)) };

        for (auto n{ 0 }; n < count; n++)
        {
            auto x1{ rx1 }, y1{ ry1 }, x2{ rx2 }, y2{ ry2 };
            if (SDL_IntersectRectAndLine(rects + n, &x1, &y1, &x2, &y2))
            {
                rx2 = x1;
                ry2 = y1;
            }
        }

        right = std::sqrt(std::pow(rx2 - rx1, 2) + std::pow(ry2 - ry1, 2));
    }
}

auto Car::render(SDL_Renderer* renderer) const -> void
{
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 0xFF);

    {
        const auto radians{ (angle + 0) * M_PI / 180.0 };

        const auto rx1{ rect.x + (rect.w / 2) * std::cos(radians) - (rect.h / 2) * std::sin(radians) };
        const auto rx2{ rect.x - (rect.w / 2) * std::cos(radians) - (rect.h / 2) * std::sin(radians) };
        const auto rx3{ rect.x + (rect.w / 2) * std::cos(radians) + (rect.h / 2) * std::sin(radians) };
        const auto rx4{ rect.x - (rect.w / 2) * std::cos(radians) + (rect.h / 2) * std::sin(radians) };

        const auto ry1{ rect.y + (rect.h / 2) * std::cos(radians) + (rect.w / 2) * std::sin(radians) };
        const auto ry2{ rect.y + (rect.h / 2) * std::cos(radians) - (rect.w / 2) * std::sin(radians) };
        const auto ry3{ rect.y - (rect.h / 2) * std::cos(radians) + (rect.w / 2) * std::sin(radians) };
        const auto ry4{ rect.y - (rect.h / 2) * std::cos(radians) - (rect.w / 2) * std::sin(radians) };

        SDL_RenderDrawLine(renderer, rx1, ry1, rx2, ry2);
        SDL_RenderDrawLine(renderer, rx1, ry1, rx3, ry3);
        SDL_RenderDrawLine(renderer, rx2, ry2, rx4, ry4);
        SDL_RenderDrawLine(renderer, rx3, ry3, rx4, ry4);
    }
    {
        const auto rx1{ rect.x + (rect.w / 2) * std::cos(radians(angle + 0)) };
        const auto rx2{ rx1 + front * std::cos(radians(angle + 0)) };

        const auto ry1{ rect.y + (rect.w / 2) * std::sin(radians(angle + 0)) };
        const auto ry2{ ry1 + front * std::sin(radians(angle + 0)) };

        SDL_RenderDrawLine(renderer, rx1, ry1, rx2, ry2);
    }
    {
        const auto radians{ (angle + 90) * M_PI / 180.0 };

        const auto rx1{ rect.x + (rect.w / 2) * std::cos(radians) };
        const auto rx2{ rx1 + left * std::cos(radians) };

        const auto ry1{ rect.y + (rect.w / 2) * std::sin(radians) };
        const auto ry2{ ry1 + left * std::sin(radians) };

        SDL_RenderDrawLine(renderer, rx1, ry1, rx2, ry2);
    }
    {
        const auto radians{ (angle - 90) * M_PI / 180.0 };

        const auto rx1{ rect.x + (rect.w / 2) * std::cos(radians) };
        const auto rx2{ rx1 + right * std::cos(radians) };

        const auto ry1{ rect.y + (rect.w / 2) * std::sin(radians) };
        const auto ry2{ ry1 + right * std::sin(radians) };

        SDL_RenderDrawLine(renderer, rx1, ry1, rx2, ry2);
    }
}

auto Car::distance() const->Distance
{
    return { front,left,right };
}

auto Car::end() -> void
{

}