#include <gl/glew.h>
#include <box2d/box2d.h>

#include <iostream>
#include <cstdio>
#include <cstdint>

#include "GLRenderPoints.hpp"
#include "GLRenderLines.hpp"
#include "GLRenderTriangles.hpp"
#include "Draw.hpp"

auto Draw::create(Camera* camera) -> void
{
    this->camera = camera;

    points = new GLRenderPoints;
    points->create(camera);
    lines = new GLRenderLines;
    lines->create(camera);
    triangles = new GLRenderTriangles;
    triangles->create(camera);
}

auto Draw::destroy() -> void
{
    points->destroy();
    delete points;
    points = nullptr;

    lines->destroy();
    delete lines;
    lines = nullptr;

    triangles->destroy();
    delete triangles;
    triangles = nullptr;
}

auto Draw::drawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) -> void
{
    b2Vec2 p1 = vertices[vertexCount - 1];
    for (int32 i = 0; i < vertexCount; ++i)
    {
        b2Vec2 p2 = vertices[i];
        lines->line(p1, p2, color);
        p1 = p2;
    }
}

auto Draw::drawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) -> void
{
    const auto fillColor{ b2Color{ 0.5f * color.r, 0.5f * color.g, 0.5f * color.b, 0.5f } };

    for (int32 i = 1; i < vertexCount - 1; ++i)
    {
        triangles->triangle(vertices[0], vertices[i], vertices[i + 1], fillColor);
    }

    b2Vec2 p1 = vertices[vertexCount - 1];
    for (int32 i = 0; i < vertexCount; ++i)
    {
        b2Vec2 p2 = vertices[i];
        lines->line(p1, p2, color);
        p1 = p2;
    }
}

auto Draw::drawCircle(const b2Vec2& center, float radius, const b2Color& color) -> void
{
    const float k_segments = 16.0f;
    const float k_increment = 2.0f * b2_pi / k_segments;
    float sinInc = sinf(k_increment);
    float cosInc = cosf(k_increment);
    b2Vec2 r1(1.0f, 0.0f);
    b2Vec2 v1 = center + radius * r1;
    for (int32 i = 0; i < k_segments; ++i)
    {
        // Perform rotation to aauto additional trigonometry. -> void
        b2Vec2 r2;
        r2.x = cosInc * r1.x - sinInc * r1.y;
        r2.y = sinInc * r1.x + cosInc * r1.y;
        b2Vec2 v2 = center + radius * r2;
        lines->line(v1, v2, color);
        r1 = r2;
        v1 = v2;
    }
}

auto Draw::drawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) -> void
{
    const float k_segments = 16.0f;
    const float k_increment = 2.0f * b2_pi / k_segments;
    float sinInc = sinf(k_increment);
    float cosInc = cosf(k_increment);
    b2Vec2 v0 = center;
    b2Vec2 r1(cosInc, sinInc);
    b2Vec2 v1 = center + radius * r1;
    b2Color fillColor(0.5f * color.r, 0.5f * color.g, 0.5f * color.b, 0.5f);
    for (int32 i = 0; i < k_segments; ++i)
    {
        // Perform rotation to aauto additional trigonometry. -> void
        b2Vec2 r2;
        r2.x = cosInc * r1.x - sinInc * r1.y;
        r2.y = sinInc * r1.x + cosInc * r1.y;
        b2Vec2 v2 = center + radius * r2;
        triangles->triangle(v0, v1, v2, fillColor);
        r1 = r2;
        v1 = v2;
    }

    r1.Set(1.0f, 0.0f);
    v1 = center + radius * r1;
    for (int32 i = 0; i < k_segments; ++i)
    {
        b2Vec2 r2;
        r2.x = cosInc * r1.x - sinInc * r1.y;
        r2.y = sinInc * r1.x + cosInc * r1.y;
        b2Vec2 v2 = center + radius * r2;
        lines->line(v1, v2, color);
        r1 = r2;
        v1 = v2;
    }

    // Draw a line fixed in the circle to animate rotation.
    b2Vec2 p = center + radius * axis;
    lines->line(center, p, color);
}

auto Draw::drawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) -> void
{
    lines->line(p1, p2, color);
}

auto Draw::drawPoint(const b2Vec2& p, float size, const b2Color& color) -> void
{
    points->point(p, color, size);
}

auto Draw::drawString(int x, int y, const char* string, ...) -> void
{
    //if (showUI == false)
    //{
    //	return;
    //}
    //
    //va_list arg;
    //va_start(arg, string);
    //ImGui::Begin("Overlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
    //ImGui::SetCursorPos(ImVec2(float(x), float(y)));
    //ImGui::TextColoredV(ImColor(230, 153, 153, 255), string, arg);
    //ImGui::End();
    //va_end(arg);
}

auto Draw::drawString(const b2Vec2& pw, const char* string, ...) -> void
{
    //b2Vec2 ps = g_camera.ConvertWorldToScreen(pw);
    //
    //va_list arg;
    //va_start(arg, string);
    //ImGui::Begin("Overlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
    //ImGui::SetCursorPos(ImVec2(ps.x, ps.y));
    //ImGui::TextColoredV(ImColor(230, 153, 153, 255), string, arg);
    //ImGui::End();
    //va_end(arg);
}

auto Draw::flush() -> void
{
    triangles->flush();
    lines->flush();
    points->flush();
}
