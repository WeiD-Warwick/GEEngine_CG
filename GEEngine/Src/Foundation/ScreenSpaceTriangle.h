#pragma once
#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "Mesh.h"
#include "Core.h"
#include "CoreMath.h"
#include "PSOManager.h"
#include <cassert>
#include <iostream>
#include "ConstantBuffer.h"
#include "Timer.h"
#include "ShaderManager.h"

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
        vertexShader = ShaderManager::loadAndCompile("Src/Shaders/VS_Triangle.hlsl", "VS", "vs_5_0");
        pixelShader = ShaderManager::loadAndCompile("Src/Shaders/PS_Triangle.hlsl", "PS", "ps_5_0");

        // Layout
        D3D12_INPUT_ELEMENT_DESC inputLayout[2];
        D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;

        inputLayout[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
        inputLayout[1] = { "COLOUR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
        inputLayoutDesc.NumElements = 2;
        inputLayoutDesc.pInputElementDescs = inputLayout;

        // Constant buffer
        constBufferCPU.time = 0.0f;
        constantBuffer.init(core, sizeof(ConstantBufferCPU), 2);

        // PSO
        psos.createPSO(core, "Triangle", vertexShader, pixelShader, inputLayoutDesc);

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

        core->beginRenderPass();
        constantBuffer.update(&constBufferCPU, sizeof(ConstantBufferCPU), core->frameIndex());
        core->getCommandList()
            ->SetGraphicsRootConstantBufferView(1, constantBuffer.getGPUAddress(core->frameIndex()));
        psos.bind(core, "Triangle");
        m.draw(core);
    }
};