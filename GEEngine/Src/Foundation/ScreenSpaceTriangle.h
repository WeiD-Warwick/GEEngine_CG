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

class ScreenSpaceTriangle {
    Mesh mesh;
    std::string shaderName;

public:
    void init(Core* core, PSOManager* psoManager, ShaderManager* shaderManager) {

        // Primitive
        PRIM_VERTEX vertices[3];
        vertices[0].position = Vec3(0, 0.5f, 0);
        vertices[0].Colour = Colour(0, 1.0f, 0);
        vertices[1].position = Vec3(-0.5f, -0.5f, 0);
        vertices[1].Colour = Colour(1.0f, 0, 0);
        vertices[2].position = Vec3(0.5f, -0.5f, 0);
        vertices[2].Colour = Colour(0, 0, 1.0f);

        unsigned int indices[3] = { 0, 1, 2 };
        mesh.init(core, &vertices[0], sizeof(PRIM_VERTEX), 3, indices, 3);

        shaderName = "TriangleShader";
        std::string vsPath = "Src/Shaders/VS_Triangle.hlsl";
        std::string psPath = "Src/Shaders/PS_Triangle.hlsl";

        shaderManager->loadShader(core, shaderName, vsPath, psPath);

        // Layout
        D3D12_INPUT_ELEMENT_DESC inputLayout[2];
        D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;

        inputLayout[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
        inputLayout[1] = { "COLOUR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
        inputLayoutDesc.NumElements = 2;
        inputLayoutDesc.pInputElementDescs = inputLayout;

        psoManager->createPSO(
            core,
            "TrianglePSO",
            shaderManager->find(shaderName)->vs,
            shaderManager->find(shaderName)->ps,
            inputLayoutDesc);
    }

    void draw(Core* core, PSOManager* psoManager, ShaderManager* shaderManager, float time) {
        float WIDTH = 1024;
        float HEIGHT = 1024;
        Vec4 lights[4] = {};
        for (int i = 0; i < 4; i++) {
            float angle = time + (i * M_PI / 2.0f);
            lights[i] = Vec4(WIDTH / 2.0f + (cosf(angle) * (WIDTH * 0.1f)),
                             HEIGHT / 2.0f + (sinf(angle) * (HEIGHT * 0.1f)),
                             0, 0);
        }

        shaderManager->updateConstantPS(shaderName, "bufferName", "time", &time);
        shaderManager->updateConstantPS(shaderName, "bufferName", "lights", &lights);

        shaderManager->apply(core, shaderName);
        psoManager->bind(core, "TrianglePSO");
        mesh.draw(core);
    }
};