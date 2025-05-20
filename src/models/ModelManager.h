#pragma once

#include "ModelLoader.h"
#include <unordered_map>
#include <filesystem>

class ModelManager {
private:
    std::unordered_map<std::string, std::shared_ptr<Model3D>> loadedModels;

public:
    ModelManager() = default;

    std::shared_ptr<Model3D> loadModel(const std::string& filePath) {

        if (loadedModels.find(filePath) != loadedModels.end()) {
            return loadedModels[filePath];
        }

        std::filesystem::path path(filePath);
        std::string extension = path.extension().string();

        auto loader = ModelLoader::createLoader(extension);

        auto model = loader->loadModel(filePath);

        loadedModels[filePath] = model;

        return model;
    }

    void clearCache() {
        loadedModels.clear();
    }
};
