#pragma once

#include "root.hpp"
#include "visualdebug/visualdebug_renderer.hpp"
#include "visualdebug/cube.hpp"
#include "visualdebug/sphere.hpp"

inline VisualDebugRenderer* VisualDebug() {
    return Root::Instance().GetVisualDebugRenderer();
}
