#pragma once
#include <vector>
#include <array>

class Mesh {
public:
    struct Vertex {
        float x, y, z;
        float nx, ny, nz;
        float u, v;
    };

    struct Face {
        std::vector<size_t> vertexIndices;
        std::vector<size_t> normalIndices;
        std::vector<size_t> texCoordIndices;
    };

private:
    std::string name;
    std::vector<Vertex> vertices;
    std::vector<Face> faces;

    std::vector<std::array<float, 3>> positions;
    std::vector<std::array<float, 3>> normals;
    std::vector<std::array<float, 2>> texCoords;

public:
    explicit Mesh(std::string meshName) : name(std::move(meshName)) {}

    const std::string& getName() const { return name; }
    const std::vector<Vertex>& getVertices() const { return vertices; }
    const std::vector<Face>& getFaces() const { return faces; }

    void addPosition(float x, float y, float z) {
        positions.push_back({ x, y, z });
    }

    void addNormal(float nx, float ny, float nz) {
        normals.push_back({ nx, ny, nz });
    }

    void addTexCoord(float u, float v) {
        texCoords.push_back({ u, v });
    }

    void addFace(const Face& face) {
        faces.push_back(face);
    }

    void processVertices() {
        vertices.clear();

        for (const auto& face : faces) {
            size_t vertexCount = face.vertexIndices.size();

            for (size_t i = 0; i < vertexCount; ++i) {
                Vertex vertex{};

                size_t posIndex = face.vertexIndices[i];
                if (posIndex < positions.size()) {
                    vertex.x = positions[posIndex][0];
                    vertex.y = positions[posIndex][1];
                    vertex.z = positions[posIndex][2];
                }

                if (i < face.texCoordIndices.size()) {
                    size_t texIndex = face.texCoordIndices[i];
                    if (texIndex < texCoords.size()) {
                        vertex.u = texCoords[texIndex][0];
                        vertex.v = texCoords[texIndex][1];
                    }
                }

                if (i < face.normalIndices.size()) {
                    size_t normIndex = face.normalIndices[i];
                    if (normIndex < normals.size()) {
                        vertex.nx = normals[normIndex][0];
                        vertex.ny = normals[normIndex][1];
                        vertex.nz = normals[normIndex][2];
                    }
                }

                vertices.push_back(vertex);
            }
        }
    }
};