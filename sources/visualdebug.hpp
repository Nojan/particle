#pragma once

#include "root.hpp"
#include "visualdebug/visualdebug_renderer.hpp"
#include "visualdebug/cube.hpp"
#include "visualdebug/sphere.hpp"
#include "visualdebug/vdb_geometry.hpp"
#include "visualdebug/VisualDebugBone.hpp"
#include "visualdebug/VisualDebugBoundingBoxCommand.h"

inline VisualDebugRenderer* VisualDebug() {
    return Root::Instance().GetVisualDebugRenderer();
}
