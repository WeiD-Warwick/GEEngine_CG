#pragma once
#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <vector>
#include "VertexLayoutCache.h"
#include "Mesh.h"
#include "CoreMath.h"
#include <fstream>
#include <sstream>
#include <string>
#include "PSOManager.h"
#include "ConstantBuffer.h"
#include "Timer.h"
#include "Layout.h"
#include "ShaderManager.h"

class Plane {
    Mesh mesh;

    ID3DBlob* vertexShader = nullptr;
    ID3DBlob* pixelShader = nullptr;
    PSOManager psos;

    ShaderManager shaderMgr;

    ConstantBuffer psConstantBuffer;
    ConstantBufferCPU psCB_CPU{};

    Timer timer;

public:
    ~Plane() {}

    Plane(Core* core)
    {

        // Create Vertex and index
        std::vector<STATIC_VERTEX> vertices;
        vertices.push_back(addVertex(Vec3(-15, 0, -15), Vec3(0, 1, 0), 0, 0));
        vertices.push_back(addVertex(Vec3(15, 0, -15), Vec3(0, 1, 0), 1, 0));
        vertices.push_back(addVertex(Vec3(-15, 0, 15), Vec3(0, 1, 0), 0, 1));
        vertices.push_back(addVertex(Vec3(15, 0, 15), Vec3(0, 1, 0), 1, 1));

        std::vector<unsigned int> indices;
        indices.push_back(2); indices.push_back(1); indices.push_back(0);
        indices.push_back(1); indices.push_back(2); indices.push_back(3);

        mesh.init(core, vertices, indices);

        // Compile Shader
        std::string vsCode = ReadShader("Src/Shaders/VS.hlsl");
        std::string psCode = ReadShader("Src/Shaders/PS.hlsl");

        vertexShader = Compile(vsCode, "VS", "vs_5_0");
        pixelShader = Compile(psCode, "PS", "ps_5_0");

        // reflection
        ID3D12ShaderReflection* vsRefl = nullptr;
        D3DReflect(
            vertexShader->GetBufferPointer(),
            vertexShader->GetBufferSize(),
            IID_PPV_ARGS(&vsRefl));

        D3D12_SHADER_DESC vsDesc;
        vsRefl->GetDesc(&vsDesc);

        ConstantBuffer vsCB;

        for (UINT i = 0; i < vsDesc.ConstantBuffers; i++)
        {
            ID3D12ShaderReflectionConstantBuffer* cb = vsRefl->GetConstantBufferByIndex(i);
            D3D12_SHADER_BUFFER_DESC cbDesc;
            cb->GetDesc(&cbDesc);

            vsCB.name = cbDesc.Name;
            unsigned int totalSize = 0;

            for (UINT j = 0; j < cbDesc.Variables; j++)
            {
                ID3D12ShaderReflectionVariable* var = cb->GetVariableByIndex(j);
                D3D12_SHADER_VARIABLE_DESC vDesc;
                var->GetDesc(&vDesc);

                ConstantBufferVariable varInfo;
                varInfo.offset = vDesc.StartOffset;
                varInfo.size = vDesc.Size;

                vsCB.constantBufferData.insert({ vDesc.Name, varInfo });
                totalSize += varInfo.size;
            }

            vsCB.cbSizeInBytes = totalSize;
        }

        vsRefl->Release();

        vsCB.init(core, vsCB.cbSizeInBytes, 2);

        // ShaderManager register
        shaderMgr.registerVSConstantBuffer("StaticModel", vsCB, "staticMeshBuffer");

        // PS Constant Buffer
        psCB_CPU.time = 0;
        psConstantBuffer.init(core, sizeof(ConstantBufferCPU), 2);


        // PSO
        psos.createPSO(core, "StaticModel",
            vertexShader, pixelShader,
            VertexLayoutCache::getStaticLayout());
    }

    std::string ReadShader(const std::string& filename)
    {
        std::ifstream file(filename);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    ID3DBlob* Compile(const std::string& src, const char* entry, const char* profile)
    {
        ID3DBlob* shader;
        ID3DBlob* error;

        HRESULT hr = D3DCompile(
            src.c_str(), strlen(src.c_str()),
            NULL, NULL, NULL,
            entry, profile,
            0, 0,
            &shader, &error);

        if (FAILED(hr))
        {
            OutputDebugStringA((char*)error->GetBufferPointer());
            error->Release();
            return nullptr;
        }

        if (error) error->Release();
        return shader;
    }

    void draw(Core* core)
    {
        float dt = timer.dt();
        psCB_CPU.time += dt;

        // Light
        int WIDTH = core->_width;
        int HEIGHT = core->_height;

        for (int i = 0; i < 4; i++) {
            float angle = psCB_CPU.time + (i * float(M_PI) / 2.0f);
            float x = WIDTH / 2.0f + cosf(angle) * WIDTH * 0.3f;
            float y = HEIGHT / 2.0f + sinf(angle) * HEIGHT * 0.3f;
            psCB_CPU.lights[i] = Vec4(x, y, 0, 0);
        }


        // Implement camera
        Matrix W = Matrix::Identity();

        Matrix view = Matrix::LookAt(
            Vec3(11 * cos(psCB_CPU.time), 5, 11 * sin(psCB_CPU.time)),
            Vec3(0, 0, 0),
            Vec3(0, 1, 0));

        Matrix proj = Matrix::Perspective(
            60.0f * M_PI / 180.0f,
            float(WIDTH) / float(HEIGHT),
            0.1f, 100.0f);

        Matrix VP = proj * view;


        // VS Update
        shaderMgr.updateConstantVS("StaticModel", "staticMeshBuffer", "W", &W);
        shaderMgr.updateConstantVS("StaticModel", "staticMeshBuffer", "VP", &VP);


        // PS constant buffer
        int frame = core->frameIndex();
        psConstantBuffer.update(&psCB_CPU, sizeof(ConstantBufferCPU), frame);


        // Render
        core->beginRenderPass();

        shaderMgr.bindVS(core, "StaticModel");

        core->getCommandList()->SetGraphicsRootConstantBufferView(
            1, psConstantBuffer.getGPUAddress(frame));

        psos.bind(core, "StaticModel");

        mesh.draw(core);
    }
};