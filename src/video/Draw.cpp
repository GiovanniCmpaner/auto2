#include <gl/glew.h>
#include <box2d/box2d.h>

#include <iostream>
#include <cstdio>
#include <cstdint>

#include "GLRenderPoints.hpp"
#include "GLRenderLines.hpp"
#include "GLRenderTriangles.hpp"
#include "Draw.hpp"

Draw::Draw(std::shared_ptr<Camera> camera)
{
    this->camera = camera;
    this->points = std::make_unique<GLRenderPoints>(camera);
    this->lines = std::make_unique<GLRenderLines>(camera);
    this->triangles = std::make_unique<GLRenderTriangles>(camera);

    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
    glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, nullptr, GL_TRUE );
    glDebugMessageCallback(Draw::debugOutput, this);
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

auto Draw::debugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) -> void
{
    std::cerr << "-------------------------------------------------------------------------------" << std::endl;
    std::cerr << "ID: " << id << std::endl;
    {
        auto text = [](GLenum source) -> const char*
        {
            switch (source)
            {
                case GL_DEBUG_SOURCE_API:             return "API";
                case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "Window System";
                case GL_DEBUG_SOURCE_SHADER_COMPILER: return "Shader Compiler";
                case GL_DEBUG_SOURCE_THIRD_PARTY:     return "Third Party";
                case GL_DEBUG_SOURCE_APPLICATION:     return "Application";
                case GL_DEBUG_SOURCE_OTHER:           return "Other";
                default:                              return "Unknown";
            }
        };
        std::cerr << "Source: " << text(source) << std::endl;
    }
    {
        auto text = [](GLenum type) -> const char*
        {
            switch (type)
            {
                case GL_DEBUG_TYPE_ERROR:               return "Error";
                case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated Behaviour";
                case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "Undefined Behaviour";
                case GL_DEBUG_TYPE_PORTABILITY:         return "Portability";
                case GL_DEBUG_TYPE_PERFORMANCE:         return "Performance";
                case GL_DEBUG_TYPE_MARKER:              return "Marker";
                case GL_DEBUG_TYPE_PUSH_GROUP:          return "Push Group";
                case GL_DEBUG_TYPE_POP_GROUP:           return "Pop Group";
                case GL_DEBUG_TYPE_OTHER:               return "Other";
                default:                                return "Unknown";
            }
        };
        std::cerr << "Type: " << text(type) << std::endl;
    }
    {
        auto text = [](GLenum severity) -> const char*
        {
            switch (severity)
            {
                case GL_DEBUG_SEVERITY_HIGH:         return "High";
                case GL_DEBUG_SEVERITY_MEDIUM:       return "Medium";
                case GL_DEBUG_SEVERITY_LOW:          return "Low";
                case GL_DEBUG_SEVERITY_NOTIFICATION: return "Notification";
                default:                             return "Unknown";
            }
        };
        std::cerr << "Severity: " << text(severity) << std::endl;
    }
    std::cerr << "Message: " << message << std::endl;
    std::cerr << std::endl;
}