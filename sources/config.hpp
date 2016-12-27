#pragma once

#ifdef __EMSCRIPTEN__
#define WEBGL
#endif

#ifndef WEBGL
#define IMGUI_ENABLE
#endif
