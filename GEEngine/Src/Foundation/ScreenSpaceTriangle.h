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
#include <cassert>
#include <iostream>
#include "Layout.h"
#include "ConstantBuffer.h"
#include "Timer.h"

struct PRIM_VERTEX {
public:
    Vec3 position;
    Colour Colour;
};

struct alignas(16) ConstantBufferCPU {
public:
    float time;
    float padding[3];
    Vec4 lights[4];
};

class ScreenSpaceTriangle {
    Mesh m;

    ID3DBlob* vertexShader = nullptr;
    ID3DBlob* pixelShader = nullptr;
    ID3D12RootSignature* rootSignature = nullptr;
    PSOManager psos;

    // Constant Buffer
    ConstantBuffer constantBuffer;
    ConstantBufferCPU constBufferCPU{};

    Timer timer;
public:
    ScreenSpaceTriangle(Core* core) {

        // Primitive
        PRIM_VERTEX vertices[3];
        vertices[0].position = Vec3(0, 1.0f, 0);
        vertices[0].Colour = Colour(0, 1.0f, 0);
        vertices[1].position = Vec3(-1.0f, -1.0f, 0);
        vertices[1].Colour = Colour(1.0f, 0, 0);
        vertices[2].position = Vec3(1.0f, -1.0f, 0);
        vertices[2].Colour = Colour(0, 0, 1.0f);

        unsigned int indices[3] = { 0, 1, 2 };
        m.init(core, &vertices[0], sizeof(PRIM_VERTEX), 3, indices, 3);

        // Compile shaders
        std::string vsCode = ReadShader("Src/Shaders/VS_Triangle.hlsl");
        std::string psCode = ReadShader("Src/Shaders/PS_Triangle.hlsl");

        vertexShader = Compile(vsCode, "VS", "vs_5_0");
        pixelShader = Compile(psCode, "PS", "ps_5_0");

        // PS Constant Buffer Reflection
        ID3D12ShaderReflection* reflection = nullptr;
        D3DReflect(
            pixelShader->GetBufferPointer(),
            pixelShader->GetBufferSize(),
            IID_PPV_ARGS(&reflection));

        D3D12_SHADER_DESC shaderDesc;
        reflection->GetDesc(&shaderDesc);

        // Iterate over constant buffers
        for (UINT i = 0; i < shaderDesc.ConstantBuffers; i++) {
            // Get details about i’th constant buffer
            ConstantBuffer buffer;
            ID3D12ShaderReflectionConstantBuffer* cb = reflection->GetConstantBufferByIndex(i);
            D3D12_SHADER_BUFFER_DESC cbDesc;
            cb->GetDesc(&cbDesc);
            buffer.name = cbDesc.Name;
            unsigned int totalSize = 0;

            // Iterate over variables in constant buffer
            for (int j = 0; j < cbDesc.Variables; j++) {
                ID3D12ShaderReflectionVariable* var = cb->GetVariableByIndex(j);
                D3D12_SHADER_VARIABLE_DESC vDesc;
                var->GetDesc(&vDesc);
                ConstantBufferVariable bufferVariable;
                bufferVariable.offset = vDesc.StartOffset;
                bufferVariable.size = vDesc.Size;
                buffer.constantBufferData.insert({ vDesc.Name, bufferVariable });
                totalSize += bufferVariable.size;
            }
            constantBuffer.cbSizeInBytes = totalSize;
        }

        reflection->Release();

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


        // Layout
        Layout layout;
        layout.init();

        // Constant buffer
        constBufferCPU.time = 0.0f;
        constantBuffer.init(core, sizeof(ConstantBufferCPU), 2);

        // PSO
        psos.createPSO(core, "Triangle", vertexShader, pixelShader, layout.inputLayoutDesc);

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

    void draw(Core* core) {
        float dt = timer.dt();
        constBufferCPU.time += dt;

        int WIDTH = core->_width;
        int HEIGHT = core->_height;

        for (int i = 0; i < 4; i++)
        {
            float angle = constBufferCPU.time + (i * float(M_PI) / 2.0f);
            float x = WIDTH / 2.0f + (cosf(angle) * (WIDTH * 0.3f));
            float y = HEIGHT / 2.0f + (sinf(angle) * (HEIGHT * 0.3f));
            constBufferCPU.lights[i] = Vec4(x, y, 0.0f, 0.0f);
        }

        int frame = core->frameIndex();

        constantBuffer.update(&constBufferCPU, sizeof(ConstantBufferCPU), frame);

        core->beginRenderPass();

        // Root Signature
        core->getCommandList()->SetGraphicsRootConstantBufferView(
            0, constantBuffer.getGPUAddress(frame));

        psos.bind(core, "Triangle");
        m.draw(core);
    }
};