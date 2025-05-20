#pragma once
#include <memory>
#include "Model3D.h"

class ModelLoader {
public:
    virtual ~ModelLoader() = default;

    virtual std::shared_ptr<Model3D> loadModel(const std::string& filePath) = 0;

    virtual bool supportsExtension(const std::string& extension) const = 0;

    static std::shared_ptr<ModelLoader> createLoader(const std::string& extension);
};