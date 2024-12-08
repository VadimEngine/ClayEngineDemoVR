#pragma once
// standard lib
#include <vector>
// third party
// project
#include "Core/Graphics/Mesh.h"
#include "Core/Graphics/Shader.h"

class Model {
public:
    Model();

    void addMeshes(std::vector<Mesh>&& meshes);

    void render(const Shader& shader);

    std::vector<Mesh> mMeshes_;

};


