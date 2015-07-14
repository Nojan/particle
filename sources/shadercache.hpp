#pragma once

#include "ressourcecache.hpp"
#include "shader.hpp"
#include "shader_loader.hpp"

class ShaderCache : public RessourceCache<ShaderProgram>
{
protected:
    std::shared_ptr<ShaderProgram> load(const std::string& name) const override
    {
        char shader_path[2][2048];
        sprintf(shader_path[0], "../shaders/%s.vertexshader", name.c_str());
        sprintf(shader_path[1], "../shaders/%s.fragmentshader", name.c_str());
        GLuint shaderId = LoadShaders(shader_path[0], shader_path[1]);
        return std::shared_ptr<ShaderProgram>(new ShaderProgram(shaderId));
    }
};
