#pragma once
#include <d3d12.h>
#include "Core.h"
#include <map>

struct ConstantBufferVariable
{
    unsigned int offset;
    unsigned int size;
};

class ConstantBuffer {

public:
    ID3D12Resource* constantBuffer;
    unsigned char* buffer;
    unsigned int cbSizeInBytes;
    unsigned int maxDrawCalls;
    unsigned int offsetIndex;

    // Variable Name : Variable Offset from start and size
    std::string name;
    std::map<std::string, ConstantBufferVariable> constantBufferData;


    void init(Core* core, unsigned int sizeInBytes, int frames)
    {
        cbSizeInBytes = (sizeInBytes + 255) & ~255;  // 256 ??

        HRESULT hr;
        D3D12_HEAP_PROPERTIES heapprops = {};
        heapprops.Type = D3D12_HEAP_TYPE_UPLOAD;
        heapprops.CreationNodeMask = 1;
        heapprops.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC cbDesc = {};
        cbDesc.Width = cbSizeInBytes * frames;
        cbDesc.Height = 1;
        cbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        cbDesc.DepthOrArraySize = 1;
        cbDesc.MipLevels = 1;
        cbDesc.SampleDesc.Count = 1;
        cbDesc.SampleDesc.Quality = 0;
        cbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        core->device->CreateCommittedResource(&heapprops, D3D12_HEAP_FLAG_NONE, &cbDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
            IID_PPV_ARGS(&constantBuffer));
        constantBuffer->Map(0, NULL, (void**)&buffer);

    }

    void update(std::string name, void* data)
    {
        ConstantBufferVariable cbVariable = constantBufferData[name];
        unsigned int offset = offsetIndex * cbSizeInBytes;
        memcpy(&buffer[offset + cbVariable.offset], data, cbVariable.size);
    }

    D3D12_GPU_VIRTUAL_ADDRESS getGPUAddress() const
    {
        return (constantBuffer->GetGPUVirtualAddress() + (offsetIndex * cbSizeInBytes));
    }

    D3D12_GPU_VIRTUAL_ADDRESS getGPUAddress(int frame) const
    {
        return constantBuffer->GetGPUVirtualAddress() + (frame * cbSizeInBytes);
    }

    void next() {
        offsetIndex++;
        if (offsetIndex >= maxDrawCalls)
        {
            offsetIndex = 0;
        }
    }

    void apply(Core* core) {
        for (int i = 0; i < vsConstantBuffers.size(); i++)
        {
            core->getCommandList()->SetGraphicsRootConstantBufferView(i, vsConstantBuffers[i].getGPUAddress());
            vsConstantBuffers[i].next();
        }
        for (int i = 0; i < psConstantBuffers.size(); i++)
        {
            core->getCommandList()->SetGraphicsRootConstantBufferView(i, psConstantBuffers[i].getGPUAddress());
            psConstantBuffers[i].next();
        }
    }


};
