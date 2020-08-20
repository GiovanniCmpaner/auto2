#include <GL/glew.h>
#include <string>
#include <iostream>

#include "Utilities.hpp"

namespace Utilities
{
    auto createShaderProgram(const std::string& vs, const std::string& fs) -> GLuint
    {
        const auto vsId{ createShaderFromString(vs, GL_VERTEX_SHADER) };
        const auto fsId{ createShaderFromString(fs, GL_FRAGMENT_SHADER) };
        if (vsId == 0 || fsId == 0)
        {
            std::cerr << "Error creating program" << std::endl;
            return 0;
        }

        const auto programId{ glCreateProgram() };
        glAttachShader(programId, vsId);
        glAttachShader(programId, fsId);
        glBindFragDataLocation(programId, 0, "color");
        glLinkProgram(programId);

        glDeleteShader(vsId);
        glDeleteShader(fsId);

        auto linked{ GL_FALSE };
        glGetProgramiv(programId, GL_LINK_STATUS, &linked);
        if (linked != GL_TRUE)
        {
            std::cerr << "Error linking program" << std::endl;
            printError(programId);
            glDeleteProgram(programId);
            return 0;
        }

        return programId;
    }

    auto createShaderFromString(const std::string& source, GLenum type) -> GLuint
    {
        const auto shaderId{ glCreateShader(type) };

        const char* sources[]{ source.data() };
        glShaderSource(shaderId, 1, sources, nullptr);
        glCompileShader(shaderId);

        auto compiled{ GL_FALSE };
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compiled);
        if (compiled != GL_TRUE)
        {
            std::cerr << "Error compiling shader of type " << type << std::endl;
            printError(shaderId);
            glDeleteShader(shaderId);
            return 0;
        }

        return shaderId;
    }

    auto printError(GLuint object) -> void
    {
        auto length{ 0 };
        if (glIsShader(object))
        {
            glGetShaderiv(object, GL_INFO_LOG_LENGTH, &length);
        }
        else if (glIsProgram(object))
        {
            glGetProgramiv(object, GL_INFO_LOG_LENGTH, &length);
        }
        else
        {
            std::cerr << "Not a shader or a program" << std::endl;
            return;
        }

        auto log{ std::string{} };
        log.reserve(length);

        if (glIsShader(object))
        {
            glGetShaderInfoLog(object, length, NULL, log.data());
        }
        else if (glIsProgram(object))
        {
            glGetProgramInfoLog(object, length, NULL, log.data());
        }

        std::cerr << log << std::endl;
    }
}