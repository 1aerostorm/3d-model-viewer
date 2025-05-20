#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Mesh.h"

class Model3D {
protected:
    std::vector<std::shared_ptr<Mesh>> meshes;
    std::string name;

public:
    explicit Model3D(std::string modelName) : name(std::move(modelName)) {}
    virtual ~Model3D() = default;

    const std::string& getName() const { return name; }
    const std::vector<std::shared_ptr<Mesh>>& getMeshes() const { return meshes; }
	void addMesh(std::shared_ptr<Mesh> mesh) {
		meshes.push_back(std::move(mesh));
	}
};
