#pragma once

#include "ressourcecache.hpp"
#include "renderableMesh.hpp"

class MeshCache : public RessourceCache<Mesh>
{
protected:
    std::shared_ptr<Mesh> load(const std::string& name) const override
    {
        return std::shared_ptr<Mesh>(new Mesh(name.c_str()));
    }
};
