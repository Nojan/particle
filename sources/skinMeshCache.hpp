#pragma once

#include "ressourcecache.hpp"
#include "armature.hpp"
#include "ressource_compiler_armature.hpp"

class SkinMeshCache : public RessourceCache<SkinMesh>
{
protected:
    std::shared_ptr<SkinMesh> load(const std::string& name) const override
    {
        std::shared_ptr<Armature> armature;
        armature.reset(new Armature());
        std::shared_ptr<SkinMesh> skinMesh;
        skinMesh.reset(new SkinMesh());
        ressource_compiler::compile_armature(name.c_str(), *armature, *skinMesh);
        skinMesh->mArmature = armature;

        return skinMesh;
    }
};
