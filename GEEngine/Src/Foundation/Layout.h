#pragma once
#include <d3d12.h>

class Layout {
public:
    D3D12_INPUT_ELEMENT_DESC inputLayout[2];
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;

    void init() {
        inputLayout[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
    D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
        inputLayout[1] = { "COLOUR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
        inputLayoutDesc.NumElements = 2;
        inputLayoutDesc.pInputElementDescs = inputLayout;
    }
};