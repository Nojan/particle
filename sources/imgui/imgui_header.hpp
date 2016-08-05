#pragma once

#include "../config.hpp"

#ifdef IMGUI_ENABLE
#include "imgui.h"
#include "imgui_impl_sdl_gl3.h"

#define IMGUI_ONLY(x) x

#define IMGUI_VAR(x,v) static auto x = v
#define IMGUI_CONST static

#else
#define IMGUI_ONLY(x)
#define IMGUI_VAR(x,v) static const auto x = v
#define IMGUI_CONST static const

#endif
