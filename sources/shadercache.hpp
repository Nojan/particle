#pragma once

#include "ressourcecache.hpp"
#include "shader.hpp"
#include "shader_loader.hpp"

class ShaderCache : public RessourceCache<ShaderProgram>
{
protected:
    std::shared_ptr<ShaderProgram> load(const std::string& name) const override
    {
        const size_t string_length_max = 2048;
        char shader_path[2][string_length_max];
        snprintf(shader_path[0], string_length_max, "../shaders/%s.vert", name.c_str());
        snprintf(shader_path[1], string_length_max, "../shaders/%s.frag", name.c_str());
        GLuint shaderId = LoadShaders(shader_path[0], shader_path[1]);
        return std::shared_ptr<ShaderProgram>(new ShaderProgram(shaderId));
    }
};
