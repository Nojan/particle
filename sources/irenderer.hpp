#pragma once

#include "config.hpp"

class IRenderer {
public:
	
	virtual void Render() = 0;

#ifdef IMGUI_ENABLE
    virtual void debug_GUI() const;
#endif

};

#ifdef IMGUI_ENABLE
inline void IRenderer::debug_GUI() const {}
#endif
