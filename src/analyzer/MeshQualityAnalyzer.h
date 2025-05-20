#pragma once
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <limits>
#include <map>
#include <utility>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class MeshQualityAnalyzer {
public:
    enum class IssueType {
        DEGENERATE_FACE,
        HIGH_ASPECT_RATIO,
        OVERLAPPING_VERTICES,
        NON_MANIFOLD_EDGE,
        INVERTED_NORMAL,
        HIGH_VALENCE_VERTEX,
        LOW_VALENCE_VERTEX,
        TEXTURE_STRETCH,
        SHARP_ANGLE
    };

    struct MeshIssue {
        IssueType type;
        size_t elementIndex;
        float severity;         
        std::vector<size_t> relatedElements;
    };

    struct MeshQualityMetrics {
        float minFaceArea;
        float maxFaceArea;
        float avgFaceArea;
        float minAspectRatio;
        float maxAspectRatio;
        float avgAspectRatio;
        float minDihedralAngle;
        float maxDihedralAngle;
        size_t nonManifoldEdgeCount;
        size_t degenerateFaceCount;
        float uvStretchFactor;
    };

private:
    const Mesh& mesh;
    float minQualityThreshold;

    std::vector<MeshIssue> issues;
    
    MeshQualityMetrics metrics;
    
    std::map<std::pair<size_t, size_t>, std::vector<size_t>> edgeFaceMap;
    
    std::vector<std::vector<size_t>> vertexConnectivity;

public:
    explicit MeshQualityAnalyzer(const Mesh& inputMesh, float threshold = 0.5f) 
        : mesh(inputMesh), minQualityThreshold(threshold) {
        metrics = {
                (std::numeric_limits<float>::max)(),
            0.0f,                               
            0.0f,
                (std::numeric_limits<float>::max)(),
            0.0f,                             
            0.0f,
                (std::numeric_limits<float>::max)(),
            0.0f,                               
            0,                                
            0,                              
            0.0f                             
        };
    }

    void analyzeQuality() {
        issues.clear();
        buildTopology();
        
        checkDegenerateFaces();
        checkAspectRatio();
        checkNonManifoldEdges();
        checkVertexValence();
        checkOverlappingVertices();
        checkNormalDirection();
        checkUVStretch();
        checkSharpAngles();
        
        calculateOverallMetrics();
    }

    const std::vector<MeshIssue>& getIssues() const {
        return issues;
    }
    
    const MeshQualityMetrics& getMetrics() const {
        return metrics;
    }
    
    std::vector<MeshIssue> getIssuesByType(IssueType type) const {
        std::vector<MeshIssue> filteredIssues;
        for (const auto& issue : issues) {
            if (issue.type == type) {
                filteredIssues.push_back(issue);
            }
        }
        return filteredIssues;
    }
    
    std::vector<MeshIssue> getIssuesBySeverity(float minSeverity) const {
        std::vector<MeshIssue> filteredIssues;
        for (const auto& issue : issues) {
            if (issue.severity >= minSeverity) {
                filteredIssues.push_back(issue);
            }
        }
        return filteredIssues;
    }
    
    std::string getQualitySummary() const {
        std::string summary = "Mesh Quality Analysis for: " + mesh.getName() + "\n";
        summary += "----------------------------------------\n";
        summary += "Total faces: " + std::to_string(mesh.getFaces().size()) + "\n";
        summary += "Total vertices: " + std::to_string(mesh.getVertices().size()) + "\n\n";
        
        summary += "Quality Metrics:\n";
        summary += "- Face Area: min=" + std::to_string(metrics.minFaceArea) + 
                   ", max=" + std::to_string(metrics.maxFaceArea) + 
                   ", avg=" + std::to_string(metrics.avgFaceArea) + "\n";
        summary += "- Aspect Ratio: min=" + std::to_string(metrics.minAspectRatio) + 
                   ", max=" + std::to_string(metrics.maxAspectRatio) + 
                   ", avg=" + std::to_string(metrics.avgAspectRatio) + "\n";
        summary += "- Dihedral Angle: min=" + std::to_string(metrics.minDihedralAngle) + 
                   ", max=" + std::to_string(metrics.maxDihedralAngle) + " degrees\n";
        summary += "- Non-manifold edges: " + std::to_string(metrics.nonManifoldEdgeCount) + "\n";
        summary += "- Degenerate faces: " + std::to_string(metrics.degenerateFaceCount) + "\n";
        summary += "- UV stretch factor: " + std::to_string(metrics.uvStretchFactor) + "\n\n";
        
        summary += "Issues Found:\n";
        std::map<IssueType, int> issueCounts;
        for (const auto& issue : issues) {
            issueCounts[issue.type]++;
        }
        
        for (const auto& [type, count] : issueCounts) {
            summary += "- " + getIssueTypeName(type) + ": " + std::to_string(count) + "\n";
        }
        
        return summary;
    }

private:
    std::string getIssueTypeName(IssueType type) const {
        switch (type) {
            case IssueType::DEGENERATE_FACE: return "Degenerate Face";
            case IssueType::HIGH_ASPECT_RATIO: return "High Aspect Ratio";
            case IssueType::OVERLAPPING_VERTICES: return "Overlapping Vertices";
            case IssueType::NON_MANIFOLD_EDGE: return "Non-manifold Edge";
            case IssueType::INVERTED_NORMAL: return "Inverted Normal";
            case IssueType::HIGH_VALENCE_VERTEX: return "High Valence Vertex";
            case IssueType::LOW_VALENCE_VERTEX: return "Low Valence Vertex";
            case IssueType::TEXTURE_STRETCH: return "Texture Stretch";
            case IssueType::SHARP_ANGLE: return "Sharp Angle";
            default: return "Unknown Issue";
        }
    }

    void buildTopology() {
        const auto& faces = mesh.getFaces();
        
        edgeFaceMap.clear();
        
        vertexConnectivity.resize(mesh.getVertices().size());
        for (auto& connections : vertexConnectivity) {
            connections.clear();
        }
        
        for (size_t faceIdx = 0; faceIdx < faces.size(); ++faceIdx) {
            const auto& face = faces[faceIdx];
            size_t vertexCount = face.vertexIndices.size();
            
            for (size_t i = 0; i < vertexCount; ++i) {
                size_t v1 = face.vertexIndices[i];
                size_t v2 = face.vertexIndices[(i + 1) % vertexCount];
                
                if (v1 > v2) std::swap(v1, v2);
                
                edgeFaceMap[{v1, v2}].push_back(faceIdx);
                
                if (std::find(vertexConnectivity[v1].begin(), vertexConnectivity[v1].end(), v2) 
                    == vertexConnectivity[v1].end()) {
                    vertexConnectivity[v1].push_back(v2);
                }
                if (std::find(vertexConnectivity[v2].begin(), vertexConnectivity[v2].end(), v1) 
                    == vertexConnectivity[v2].end()) {
                    vertexConnectivity[v2].push_back(v1);
                }
            }
        }
    }

    float calculateTriangleArea(size_t v1Idx, size_t v2Idx, size_t v3Idx) const {
        const auto& vertices = mesh.getVertices();
        const auto& v1 = vertices[v1Idx];
        const auto& v2 = vertices[v2Idx];
        const auto& v3 = vertices[v3Idx];
        
        float ax = v2.x - v1.x;
        float ay = v2.y - v1.y;
        float az = v2.z - v1.z;
        
        float bx = v3.x - v1.x;
        float by = v3.y - v1.y;
        float bz = v3.z - v1.z;
        
        float cx = ay * bz - az * by;
        float cy = az * bx - ax * bz;
        float cz = ax * by - ay * bx;
        
        return 0.5f * std::sqrt(cx * cx + cy * cy + cz * cz);
    }
    
    float calculateDistance(size_t v1Idx, size_t v2Idx) const {
        const auto& vertices = mesh.getVertices();
        const auto& v1 = vertices[v1Idx];
        const auto& v2 = vertices[v2Idx];
        
        float dx = v2.x - v1.x;
        float dy = v2.y - v1.y;
        float dz = v2.z - v1.z;
        
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }
    
    float calculateAspectRatio(size_t v1Idx, size_t v2Idx, size_t v3Idx) const {
        float edge1 = calculateDistance(v1Idx, v2Idx);
        float edge2 = calculateDistance(v2Idx, v3Idx);
        float edge3 = calculateDistance(v3Idx, v1Idx);
        
        float maxEdge = (std::max)({edge1, edge2, edge3});
        
        float area = calculateTriangleArea(v1Idx, v2Idx, v3Idx);
        
        float minHeight = (area > 1e-6f) ? (2.0f * area / maxEdge) : 0.0f;
        
        return (minHeight > 1e-6f) ? (maxEdge / minHeight) : (std::numeric_limits<float>::max)();
    }
    
    float calculateDihedralAngle(size_t face1Idx, size_t face2Idx, 
                                 size_t sharedV1Idx, size_t sharedV2Idx) const {
        const auto& vertices = mesh.getVertices();
        const auto& faces = mesh.getFaces();
        
        size_t nonSharedV1 = 0;
        for (size_t idx : faces[face1Idx].vertexIndices) {
            if (idx != sharedV1Idx && idx != sharedV2Idx) {
                nonSharedV1 = idx;
                break;
            }
        }
        
        size_t nonSharedV2 = 0;
        for (size_t idx : faces[face2Idx].vertexIndices) {
            if (idx != sharedV1Idx && idx != sharedV2Idx) {
                nonSharedV2 = idx;
                break;
            }
        }
        
        const auto& v1 = vertices[sharedV1Idx];
        const auto& v2 = vertices[sharedV2Idx];
        const auto& v3 = vertices[nonSharedV1];
        const auto& v4 = vertices[nonSharedV2];
        
        float a1x = v2.x - v1.x;
        float a1y = v2.y - v1.y;
        float a1z = v2.z - v1.z;
        
        float b1x = v3.x - v1.x;
        float b1y = v3.y - v1.y;
        float b1z = v3.z - v1.z;
        
        float n1x = a1y * b1z - a1z * b1y;
        float n1y = a1z * b1x - a1x * b1z;
        float n1z = a1x * b1y - a1y * b1x;
        
        float len1 = std::sqrt(n1x * n1x + n1y * n1y + n1z * n1z);
        if (len1 > 1e-6f) {
            n1x /= len1;
            n1y /= len1;
            n1z /= len1;
        }
        
        float a2x = v2.x - v1.x;
        float a2y = v2.y - v1.y;
        float a2z = v2.z - v1.z;
        
        float b2x = v4.x - v1.x;
        float b2y = v4.y - v1.y;
        float b2z = v4.z - v1.z;
        
        float n2x = a2y * b2z - a2z * b2y;
        float n2y = a2z * b2x - a2x * b2z;
        float n2z = a2x * b2y - a2y * b2x;
        
        float len2 = std::sqrt(n2x * n2x + n2y * n2y + n2z * n2z);
        if (len2 > 1e-6f) {
            n2x /= len2;
            n2y /= len2;
            n2z /= len2;
        }
        
        float dotProduct = n1x * n2x + n1y * n2y + n1z * n2z;
        
        dotProduct = (std::max)(-1.0f, (std::min)(1.0f, dotProduct));
        
        return std::acos(dotProduct) * 180.0f / M_PI;
    }
    
    float calculateUVStretch(size_t faceIdx) const {
        const auto& faces = mesh.getFaces();
        const auto& vertices = mesh.getVertices();
        const auto& face = faces[faceIdx];
        
        if (face.vertexIndices.size() < 3 || face.texCoordIndices.size() < 3) {
            return 0.0f;
        }
        
        size_t v1Idx = face.vertexIndices[0];
        size_t v2Idx = face.vertexIndices[1];
        size_t v3Idx = face.vertexIndices[2];
        
        const auto& v1 = vertices[v1Idx];
        const auto& v2 = vertices[v2Idx];
        const auto& v3 = vertices[v3Idx];
        
        float surfaceArea = calculateTriangleArea(v1Idx, v2Idx, v3Idx);
        
        float u1 = v1.u, v1uv = v1.v;
        float u2 = v2.u, v2uv = v2.v;
        float u3 = v3.u, v3uv = v3.v;
        
        float uvArea = 0.5f * std::abs((u2 - u1) * (v3uv - v1uv) - (u3 - u1) * (v2uv - v1uv));
        
        if (surfaceArea < 1e-6f || uvArea < 1e-6f) {
            return 0.0f;
        }
        
        return (std::max)(surfaceArea / uvArea, uvArea / surfaceArea);
    }

    void checkDegenerateFaces() {
        const auto& faces = mesh.getFaces();
        float totalArea = 0.0f;
        float minArea = (std::numeric_limits<float>::max)();
        float maxArea = 0.0f;
        
        for (size_t faceIdx = 0; faceIdx < faces.size(); ++faceIdx) {
            const auto& face = faces[faceIdx];
            
            if (face.vertexIndices.size() < 3) continue;
            
            float area = calculateTriangleArea(
                face.vertexIndices[0], 
                face.vertexIndices[1], 
                face.vertexIndices[2]
            );
            
            totalArea += area;
            minArea = (std::min)(minArea, area);
            maxArea = (std::max)(maxArea, area);
            
            const float DEGENERATE_THRESHOLD = 1e-5f;
            if (area < DEGENERATE_THRESHOLD) {
                MeshIssue issue;
                issue.type = IssueType::DEGENERATE_FACE;
                issue.elementIndex = faceIdx;
                issue.severity = 1.0f - (area / DEGENERATE_THRESHOLD);
                issue.relatedElements = face.vertexIndices;
                issues.push_back(issue);
                metrics.degenerateFaceCount++;
            }
        }
        
        if (faces.size() > 0) {
            metrics.minFaceArea = minArea;
            metrics.maxFaceArea = maxArea;
            metrics.avgFaceArea = totalArea / faces.size();
        }
    }
    
    void checkAspectRatio() {
        const auto& faces = mesh.getFaces();
        float totalAspectRatio = 0.0f;
        float minAspectRatio = (std::numeric_limits<float>::max)();
        float maxAspectRatio = 0.0f;
        
        for (size_t faceIdx = 0; faceIdx < faces.size(); ++faceIdx) {
            const auto& face = faces[faceIdx];
            
            if (face.vertexIndices.size() < 3) continue;
            
            float aspectRatio = calculateAspectRatio(
                face.vertexIndices[0], 
                face.vertexIndices[1], 
                face.vertexIndices[2]
            );
            
            totalAspectRatio += aspectRatio;
            minAspectRatio = (std::min)(minAspectRatio, aspectRatio);
            maxAspectRatio = (std::max)(maxAspectRatio, aspectRatio);
            
            const float HIGH_ASPECT_THRESHOLD = 10.0f;
            if (aspectRatio > HIGH_ASPECT_THRESHOLD) {
                float normalizedSeverity = (std::min)(1.0f, (aspectRatio - HIGH_ASPECT_THRESHOLD) / 30.0f);
                
                MeshIssue issue;
                issue.type = IssueType::HIGH_ASPECT_RATIO;
                issue.elementIndex = faceIdx;
                issue.severity = normalizedSeverity;
                issue.relatedElements = face.vertexIndices;
                issues.push_back(issue);
            }
        }
        
        if (faces.size() > 0) {
            metrics.minAspectRatio = minAspectRatio;
            metrics.maxAspectRatio = maxAspectRatio;
            metrics.avgAspectRatio = totalAspectRatio / faces.size();
        }
    }
    
    void checkNonManifoldEdges() {
        for (const auto& [edge, facesUsingEdge] : edgeFaceMap) {
            if (facesUsingEdge.size() > 2) {
                MeshIssue issue;
                issue.type = IssueType::NON_MANIFOLD_EDGE;
                issue.elementIndex = edge.first; 
                issue.severity = (std::min)(1.0f, (facesUsingEdge.size() - 2) / 4.0f);
                issue.relatedElements = {edge.second};
                issues.push_back(issue);
                metrics.nonManifoldEdgeCount++;
            }
        }
    }
    
    void checkVertexValence() {
        const size_t LOW_VALENCE_THRESHOLD = 3;
        const size_t HIGH_VALENCE_THRESHOLD = 12;
        
        for (size_t vertexIdx = 0; vertexIdx < vertexConnectivity.size(); ++vertexIdx) {
            size_t valence = vertexConnectivity[vertexIdx].size();
            
            if (valence > 0 && valence < LOW_VALENCE_THRESHOLD) {
                MeshIssue issue;
                issue.type = IssueType::LOW_VALENCE_VERTEX;
                issue.elementIndex = vertexIdx;
                issue.severity = 1.0f - (static_cast<float>(valence) / LOW_VALENCE_THRESHOLD);
                issue.relatedElements = vertexConnectivity[vertexIdx];
                issues.push_back(issue);
            }
            
            if (valence > HIGH_VALENCE_THRESHOLD) {
                MeshIssue issue;
                issue.type = IssueType::HIGH_VALENCE_VERTEX;
                issue.elementIndex = vertexIdx;
                issue.severity = (std::min)(1.0f, (valence - HIGH_VALENCE_THRESHOLD) / 8.0f);
                issue.relatedElements = vertexConnectivity[vertexIdx];
                issues.push_back(issue);
            }
        }
    }
    
    void checkOverlappingVertices() {
        const auto& vertices = mesh.getVertices();
        const float OVERLAP_THRESHOLD = 1e-4f;
        
        for (size_t i = 0; i < vertices.size(); ++i) {
            for (size_t j = i + 1; j < vertices.size(); ++j) {
                const auto& v1 = vertices[i];
                const auto& v2 = vertices[j];
                
                float dx = v2.x - v1.x;
                float dy = v2.y - v1.y;
                float dz = v2.z - v1.z;
                float distSq = dx * dx + dy * dy + dz * dz;
                
                if (distSq < OVERLAP_THRESHOLD * OVERLAP_THRESHOLD) {
                    MeshIssue issue;
                    issue.type = IssueType::OVERLAPPING_VERTICES;
                    issue.elementIndex = i;
                    issue.severity = 1.0f - (std::sqrt(distSq) / OVERLAP_THRESHOLD);
                    issue.relatedElements = {j};
                    issues.push_back(issue);
                }
            }
        }
    }
    
    void checkNormalDirection() {
        const auto& faces = mesh.getFaces();
        const auto& vertices = mesh.getVertices();
        
        for (size_t faceIdx = 0; faceIdx < faces.size(); ++faceIdx) {
            const auto& face = faces[faceIdx];
            
            if (face.vertexIndices.size() < 3) continue;
            
            size_t v1Idx = face.vertexIndices[0];
            size_t v2Idx = face.vertexIndices[1];
            size_t v3Idx = face.vertexIndices[2];
            
            const auto& v1 = vertices[v1Idx];
            const auto& v2 = vertices[v2Idx];
            const auto& v3 = vertices[v3Idx];
            
            float e1x = v2.x - v1.x;
            float e1y = v2.y - v1.y;
            float e1z = v2.z - v1.z;
            
            float e2x = v3.x - v1.x;
            float e2y = v3.y - v1.y;
            float e2z = v3.z - v1.z;
            
            float nx = e1y * e2z - e1z * e2y;
            float ny = e1z * e2x - e1x * e2z;
            float nz = e1x * e2y - e1y * e2x;
            
            float len = std::sqrt(nx * nx + ny * ny + nz * nz);
            if (len > 1e-6f) {
                nx /= len;
                ny /= len;
                nz /= len;
            } else {
                continue;
            }
            
            float vnx = (v1.nx + v2.nx + v3.nx) / 3.0f;
            float vny = (v1.ny + v2.ny + v3.ny) / 3.0f;
            float vnz = (v1.nz + v2.nz + v3.nz) / 3.0f;
            
            float vnLen = std::sqrt(vnx * vnx + vny * vny + vnz * vnz);
            if (vnLen > 1e-6f) {
                vnx /= vnLen;
                vny /= vnLen;
                vnz /= vnLen;
                
                float dotProduct = nx * vnx + ny * vny + nz * vnz;
                
                if (dotProduct < 0) {
                    MeshIssue issue;
                    issue.type = IssueType::INVERTED_NORMAL;
                    issue.elementIndex = faceIdx;
                    issue.severity = (std::min)(1.0f, -dotProduct);
                    issue.relatedElements = face.vertexIndices;
                    issues.push_back(issue);
                }
            }
        }
    }
    
    void checkUVStretch() {
        const auto& faces = mesh.getFaces();
        float totalStretch = 0.0f;
        size_t stretchableCount = 0;
        
        for (size_t faceIdx = 0; faceIdx < faces.size(); ++faceIdx) {
            float stretch = calculateUVStretch(faceIdx);
            
            if (stretch > 0.0f) {
                totalStretch += stretch;
                stretchableCount++;
                
                const float HIGH_STRETCH_THRESHOLD = 4.0f;
                if (stretch > HIGH_STRETCH_THRESHOLD) {
                    MeshIssue issue;
                    issue.type = IssueType::TEXTURE_STRETCH;
                    issue.elementIndex = faceIdx;
                    issue.severity = (std::min)(1.0f, (stretch - HIGH_STRETCH_THRESHOLD) / 6.0f);
                    issue.relatedElements = faces[faceIdx].vertexIndices;
                    issues.push_back(issue);
                }
            }
        }
        
        if (stretchableCount > 0) {
            metrics.uvStretchFactor = totalStretch / stretchableCount;
        }
    }
    
    void checkSharpAngles() {
        for (const auto& [edge, facesUsingEdge] : edgeFaceMap) {
            if (facesUsingEdge.size() == 2) {
                float angle = calculateDihedralAngle(
                    facesUsingEdge[0], facesUsingEdge[1], edge.first, edge.second);
                
                if (angle < metrics.minDihedralAngle || metrics.minDihedralAngle < 0.1f) {
                    metrics.minDihedralAngle = angle;
                }
                
                if (angle > metrics.maxDihedralAngle) {
                    metrics.maxDihedralAngle = angle;
                }
                
                const float SHARP_ANGLE_THRESHOLD = 30.0f;
                if (angle < SHARP_ANGLE_THRESHOLD) {
                    MeshIssue issue;
                    issue.type = IssueType::SHARP_ANGLE;
                    issue.elementIndex = edge.first;
                    issue.severity = 1.0f - (angle / SHARP_ANGLE_THRESHOLD);
                    issue.relatedElements = {edge.second};
                    issues.push_back(issue);
                }
            }
        }
    }
    
    void calculateOverallMetrics() {
        
    }
};