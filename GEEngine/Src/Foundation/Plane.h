#pragma once
#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "Mesh.h"
#include "Core.h"
#include "CoreMath.h"
#include <fstream>
#include <sstream>
#include "PSOManager.h"
#include <iostream>
#include "ConstantBuffer.h"
#include "Timer.h"
#include "ShaderManager.h"

class Plane {
	Mesh mesh;

    ID3DBlob* vertexShader = nullptr;
    ID3DBlob* pixelShader = nullptr;

    PSOManager psos;

    ShaderManager shaders;

    ConstantBuffer constantBuffer;
    ConstantBufferVariable vsCB;

    Timer timer;

    float totalTime = 0;

public:
	Plane(Core* core) {
		std::vector<STATIC_VERTEX> vertices;
		vertices.push_back(addVertex(Vec3(-15, 0, -15), Vec3(0, 1, 0), 0, 0));
		vertices.push_back(addVertex(Vec3(15, 0, -15), Vec3(0, 1, 0), 1, 0));
		vertices.push_back(addVertex(Vec3(-15, 0, 15), Vec3(0, 1, 0), 0, 1));
		vertices.push_back(addVertex(Vec3(15, 0, 15), Vec3(0, 1, 0), 1, 1));
		std::vector<unsigned int> indices;
		indices.push_back(2); indices.push_back(1); indices.push_back(0);
		indices.push_back(1); indices.push_back(2); indices.push_back(3);
		mesh.init(core, vertices, indices);

		// Compile shaders
        vertexShader = ShaderManager::loadAndCompile("Src/Shaders/VS.hlsl", "VS", "vs_5_0");
        pixelShader = ShaderManager::loadAndCompile("Src/Shaders/PS.hlsl", "PS", "ps_5_0");

        // Reflection
        unsigned int cbSize = constantBuffer.buildFromReflection(vertexShader);

        constantBuffer.init(core, cbSize);

        shaders.addConstantBufferVS("StaticModel", "staticMeshBuffer", &constantBuffer);

        psos.createPSO(core, "Plane", vertexShader, pixelShader, VertexLayoutCache::getStaticLayout());
	}

    std::string ReadShader(const std::string& filename) {
        std::ifstream file(filename);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    ID3DBlob* Compile(const std::string& src, const char* entry, const char* profile) {
        assert(src.size() > 0);
        ID3DBlob* shader;
        ID3DBlob* status;

        HRESULT hr = D3DCompile(
            src.c_str(), strlen(src.c_str()),
            NULL, NULL, NULL,
            entry, profile,
            0, 0,
            &shader, &status
        );

        if (FAILED(hr))
        {
            OutputDebugStringA((char*)status->GetBufferPointer());
            status->Release();
            return nullptr;
        }

        if (status) status->Release();
        return shader;
    }

    void update(Core* core, const Matrix& W, const Matrix& VP)
    {
        shaders.updateConstantVS(core, "StaticModel", "staticMeshBuffer", "W", &W);
        shaders.updateConstantVS(core, "StaticModel", "staticMeshBuffer", "VP", &VP);
    }


    void draw(Core* core)
    {
        float t = timer.dt();
        totalTime += t;
        Vec3 from = Vec3(11 * cosf(totalTime), 5, 11 * sinf(totalTime));

        Matrix view = Matrix::LookAt(from, Vec3(0, 1, 0), Vec3(0, 1, 0));

        float fov = M_PI / 4.0f;
        float aspect = 1024 / float(1024);
        Matrix proj = Matrix::Perspective(fov, aspect, 0.1f, 1000.0f);

        const Matrix VP = proj * view;
        const Matrix W = Matrix::Identity();

        update(core, W, VP);

        core->beginRenderPass();
        constantBuffer.update(&vsCB, sizeof(ConstantBufferCPU), core->frameIndex());

        psos.bind(core, "Plane");
        mesh.draw(core);
    }
};