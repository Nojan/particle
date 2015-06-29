#pragma once

#include "config.hpp"
#include "shader.hpp"
#include "shader_loader.hpp"
#include "ressourcecache.hpp"

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

static ShaderCache gloShaderCache;

class IRenderer {
public:
	
	virtual void Render() = 0;

#ifdef IMGUI_ENABLE
    virtual void debug_GUI() const;
    virtual const char* debug_name() const = 0;
#endif

};

#ifdef IMGUI_ENABLE
inline void IRenderer::debug_GUI() const {}
#endif
