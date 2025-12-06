#pragma once
#include <string>
#include "../Foundation/Mesh.h"
#include "../Foundation/PSOManager.h"
#include "../Foundation/ShaderManager.h"
#include "../Foundation/VertexLayoutCache.h"

class Sphere {
    Mesh mesh;
    std::string shaderName;

public:
	void init(Core* core, PSOManager* psoManager, ShaderManager* shaderManager, int rings, int segments, float radius) {
        std::vector<STATIC_VERTEX> vertices;
        std::vector<unsigned int> indices;

        vertices = getVertices(rings, segments, radius);
        indices = getIndices(rings, segments, radius);
        mesh.init(core, vertices, indices);

        shaderName = "StaticModelUntextured";
        std::string vsPath = "Src/Shaders/VS.hlsl";
        std::string psPath = "Src/Shaders/PS.hlsl";
        shaderManager->loadShader(core, shaderName, vsPath, psPath);
        psoManager->createPSO(
            core,
            "StaticModelUntexturedPSO",
            shaderManager->find(shaderName)->vs,
            shaderManager->find(shaderName)->ps,
            VertexLayoutCache::getStaticLayout());
	}

    std::vector<STATIC_VERTEX> getVertices(int rings, int segments, float radius) {
        std::vector<STATIC_VERTEX> vertices;
        for (int lat = 0; lat <= rings; lat++) {
            float theta = lat * M_PI / rings;
            float sinTheta = sinf(theta);
            float cosTheta = cosf(theta);
            for (int lon = 0; lon <= segments; lon++) {
                float phi = lon * 2.0f * M_PI / segments;
                float sinPhi = sinf(phi);
                float cosPhi = cosf(phi);
                Vec3 position(radius * sinTheta * cosPhi, radius * cosTheta, radius * sinTheta * sinPhi);
                Vec3 normal = position.normalized();
                float tu = 1.0f - (float)lon / segments;
                float tv = 1.0f - (float)lat / rings;
                vertices.push_back(addVertex(position, normal, tu, tv));
            }
        }
        return vertices;
    }

    std::vector<unsigned int> getIndices(int rings, int segments, float radius) {
        std::vector<unsigned int> indices;
        for (int lat = 0; lat < rings; lat++) {
            for (int lon = 0; lon < segments; lon++) {
                int current = lat * (segments + 1) + lon;
                int next = current + segments + 1;
                indices.push_back(current);
                indices.push_back(next);
                indices.push_back(current + 1);
                indices.push_back(current + 1);
                indices.push_back(next);
                indices.push_back(next + 1);
            }
        }
        return indices;
    }

    void draw(Core* core, PSOManager* psos, ShaderManager* shaders, float time) {
        Matrix W;
        Matrix vp;
        Matrix p = Matrix::perspectiveLH(0.01f, 10000.0f, 1024.0f / 1024.0f, 60.0f);
        Vec3 from = Vec3(50 * cos(time), 50, 50 * sinf(time));
        Matrix v = Matrix::lookAtLH(from, Vec3(0, 0, 0), Vec3(0, 1, 0));
        vp = p * v;
        shaders->updateConstantVS(shaderName, "staticMeshBuffer", "VP", &vp);
        shaders->updateConstantVS(shaderName, "staticMeshBuffer", "W", &W);
        shaders->apply(core, shaderName);
        psos->bind(core, "StaticModelUntexturedPSO");
        mesh.draw(core);
    }
};
