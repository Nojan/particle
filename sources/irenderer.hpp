#pragma once

#include "config.hpp"
#include "shader.hpp"

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
