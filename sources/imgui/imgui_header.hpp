#pragma once

#include "../config.hpp"

#ifdef IMGUI_ENABLE
#include "imgui.hpp"
#include "imgui_impl_glfw_gl3.hpp"

#define IMGUI_ONLY(x) x

#else
#define IMGUI_ONLY(x)

#endif
