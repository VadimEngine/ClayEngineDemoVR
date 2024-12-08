#include "Model.h"

Model::Model() = default;

void Model::addMeshes(std::vector<Mesh>&& meshes) {
    mMeshes_.insert(mMeshes_.end(), std::make_move_iterator(meshes.begin()), std::make_move_iterator(meshes.end()));
}

void Model::render(const Shader& shader) {
    for (auto& each: mMeshes_) {
        each.render(shader);
    }
}