#pragma once

#include <GL/glew.h>
#include <string>

namespace Utilities
{
    auto createShaderProgram(const std::string& vs, const std::string& fs)->GLuint;

    auto createShaderFromString(const std::string& source, GLenum type)->GLuint;

    auto printError(GLuint object) -> void;
}