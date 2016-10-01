#pragma once

struct Armature;
struct SkinMesh;

namespace resource_compiler {
    void compile_armature(const char* filepath, Armature& armature, SkinMesh& skinMesh);
};
