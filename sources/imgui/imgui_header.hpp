#pragma once

#include "../config.hpp"

#ifdef IMGUI_ENABLE
#include "imgui.hpp"
#include "imgui_impl_glfw_gl3.hpp"

#define IMGUI_ONLY(x) x

#define IMGUI_VAR(x,v) static auto x = v

#else
#define IMGUI_ONLY(x)
#define IMGUI_VAR(x,v) static const auto x = v

#endif
