#include "ModelLoader.h"
#include "ObjLoader.h"

std::shared_ptr<ModelLoader> ModelLoader::createLoader(const std::string& extension) {
    auto objLoader = std::make_shared<ObjLoader>();
    if (objLoader->supportsExtension(extension)) {
        return objLoader;
    }

    throw std::runtime_error("Unsupported file extension: " + extension);
}