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
#include "Layout.h"
#include "ConstantBuffer.h"
#include "Timer.h"
#include "ShaderManager.h"

class Plane {
	Mesh mesh;

    ID3DBlob* vertexShader = nullptr;
    ID3DBlob* pixelShader = nullptr;

    PSOManager psos;

    ShaderManager shaders;

    ConstantBuffer* vsCB = nullptr;
    ConstantBuffer* psCB = nullptr;

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
		std::string vsCode = ReadShader("Src/Shaders/VS.hlsl");
		std::string psCode = ReadShader("Src/Shaders/PS.hlsl");

		vertexShader = Compile(vsCode, "VS", "vs_5_0");
		pixelShader = Compile(psCode, "PS", "ps_5_0");

        // Reflection
        vsCB = new ConstantBuffer();
        vsCB->init(core, 256);
        vsCB->buildFromReflection(vertexShader);
        shaders.addVSConstantBuffer("StaticModel", "staticMeshBuffer", vsCB);
        psCB = new ConstantBuffer();
        psCB->init(core, 256);
        psCB->buildFromReflection(pixelShader);
        shaders.addVSConstantBuffer("StaticModel", "staticMeshBuffer", psCB);

        // Root Signature
        D3D12_ROOT_PARAMETER param = {};
        param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        param.Descriptor.ShaderRegister = 0;
        param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        desc.NumParameters = 1;
        desc.pParameters = &param;

        ID3DBlob* serialized = nullptr;
        ID3DBlob* error = nullptr;
        D3D12SerializeRootSignature(
            &desc,
            D3D_ROOT_SIGNATURE_VERSION_1,
            &serialized, &error);

        core->device->CreateRootSignature(
            0,
            serialized->GetBufferPointer(),
            serialized->GetBufferSize(),
            IID_PPV_ARGS(&core->rootSignature));

        serialized->Release();

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

    void update(const Matrix& W, const Matrix& VP)
    {
        shaders.updateConstantVS("StaticModel", "staticMeshBuffer", "W", (void*)&W);
        shaders.updateConstantVS("StaticModel", "staticMeshBuffer", "VP", (void*)&VP);
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

        Matrix VP = (proj * view).transpose();
        Matrix W = Matrix::Identity().transpose();

        update(W, VP);

        core->getCommandList()->SetGraphicsRootSignature(core->rootSignature);

        shaders.applyVS(core, "StaticModel");
        shaders.applyPS(core, "StaticModel");

        psos.bind(core, "Plane");

        mesh.draw(core);
    }


};