#pragma once

#include "ModelLoader.h"
#include <filesystem>
#include <fstream>  
#include <sstream>  

class ObjLoader : public ModelLoader {
public:
    bool supportsExtension(const std::string& extension) const override {
        return extension == ".obj";
    }

    std::shared_ptr<Model3D> loadModel(const std::string& filePath) override {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filePath);
        }

        std::filesystem::path path(filePath);
        std::string modelName = path.stem().string();

        auto model = std::make_shared<Model3D>(modelName);
        auto currentMesh = std::make_shared<Mesh>("default");

        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string token;
            iss >> token;

            if (token == "v") {
                float x, y, z;
                if (iss >> x >> y >> z) {
                    currentMesh->addPosition(x, y, z);
                }
            }
            else if (token == "vn") {  
                float nx, ny, nz;
                if (iss >> nx >> ny >> nz) {
                    currentMesh->addNormal(nx, ny, nz);
                }
            }
            else if (token == "vt") {
                float u, v;
                if (iss >> u >> v) {
                    currentMesh->addTexCoord(u, v);
                }
            }
            else if (token == "f") {
                Mesh::Face face;
                std::string vertexData;

                while (iss >> vertexData) {
                    std::istringstream vertexStream(vertexData);
                    std::string indexStr;

                    if (std::getline(vertexStream, indexStr, '/')) {
                        if (!indexStr.empty()) {
                            face.vertexIndices.push_back(std::stoul(indexStr) - 1);
                        }
                    }

                    if (std::getline(vertexStream, indexStr, '/')) {
                        if (!indexStr.empty()) {
                            face.texCoordIndices.push_back(std::stoul(indexStr) - 1);
                        }
                    }

                    if (std::getline(vertexStream, indexStr, '/')) {
                        if (!indexStr.empty()) {
                            face.normalIndices.push_back(std::stoul(indexStr) - 1);
                        }
                    }
                }

                if (!face.vertexIndices.empty()) {
                    currentMesh->addFace(face);
                }
            }
            else if (token == "o" || token == "g") {
                if (!currentMesh->getFaces().empty()) {
                    currentMesh->processVertices();
                    model->addMesh(currentMesh);
                }

                std::string meshName;
                iss >> meshName;
                if (meshName.empty()) {
                    meshName = "unnamed_" + std::to_string(model->getMeshes().size());
                }
                currentMesh = std::make_shared<Mesh>(meshName);
            }
        }

        if (!currentMesh->getFaces().empty()) {
            currentMesh->processVertices();
            model->addMesh(currentMesh);
        }

        return model;
    }
};