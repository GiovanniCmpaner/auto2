#pragma once

#include <GL/glew.h>
#include <string>

namespace Shader
{
    auto createProgram(const std::string& vs, const std::string& fs)->GLuint;

    auto createFromString(const std::string& source, GLenum type)->GLuint;

    auto printError(GLuint object) -> void;

    auto debugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) -> void;
}