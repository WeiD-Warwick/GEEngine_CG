#pragma once
#include <d3d12.h>
#include <d3d12shader.h>
#include <d3dcompiler.h>
#include <map>
#include <string>
#include <vector>
#include "Core.h"
#pragma comment(lib, "d3d12.lib")


struct ConstantBufferVariable {
    unsigned int offset;
    unsigned int size;
};

class ConstantBuffer {
public:
    ID3D12Resource* constantBuffer = nullptr;
    unsigned char* buffer = nullptr;
    unsigned int cbSizeInBytes = 0;
    unsigned int maxDrawCalls = 0;
    unsigned int offsetIndex = 0;

    std::string name;
    std::map<std::string, ConstantBufferVariable> constantBufferData;

    void init(Core *core, unsigned int sizeInBytes, unsigned int _maxDrawCalls = 1024) {
        cbSizeInBytes = (sizeInBytes + 255) & ~255;
        maxDrawCalls = _maxDrawCalls;
        offsetIndex = 0;

        unsigned int totalSize = cbSizeInBytes * maxDrawCalls;

        D3D12_HEAP_PROPERTIES heapprops = {};
        heapprops.Type = D3D12_HEAP_TYPE_UPLOAD;
        heapprops.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapprops.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapprops.CreationNodeMask = 1;
        heapprops.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC cbDesc = {};
        cbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        cbDesc.Width = totalSize;
        cbDesc.Height = 1;
        cbDesc.DepthOrArraySize = 1;
        cbDesc.MipLevels = 1;
        cbDesc.SampleDesc.Count = 1;
        cbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        core->device->CreateCommittedResource(
            &heapprops,
            D3D12_HEAP_FLAG_NONE,
            &cbDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&constantBuffer)
        );

        constantBuffer->Map(0, nullptr, (void**)&buffer);
    }

    void update(void* data, unsigned int sizeInBytes, int frame)
    {
        memcpy(buffer + (frame * cbSizeInBytes), data, sizeInBytes);
    }

    void update(const std::string& name, const void* data, int frame)
    {
        ConstantBufferVariable cbVar = constantBufferData[name];
        unsigned int base = frame * cbSizeInBytes;
        memcpy(buffer + base + cbVar.offset, data, cbVar.size);
    }


    D3D12_GPU_VIRTUAL_ADDRESS getGPUAddress(int frame)
    {
        return (constantBuffer->GetGPUVirtualAddress() + (frame * cbSizeInBytes));
    }

    D3D12_GPU_VIRTUAL_ADDRESS getGPUAddress() const {
        return constantBuffer->GetGPUVirtualAddress() + (offsetIndex * cbSizeInBytes);
    }

    void next() {
        offsetIndex++;
        if (offsetIndex >= maxDrawCalls) offsetIndex = 0;
    }

    void resetFrame() {
        offsetIndex = 0;
    }

    ~ConstantBuffer() {
        if (constantBuffer) {
            constantBuffer->Unmap(0, nullptr);
            constantBuffer->Release();
            constantBuffer = nullptr;
        }
    }

    unsigned int buildFromReflection(ID3DBlob* shaderBlob)
    {
        ID3D12ShaderReflection* reflection;
        D3DReflect(
            shaderBlob->GetBufferPointer(),
            shaderBlob->GetBufferSize(),
            IID_PPV_ARGS(&reflection));
        D3D12_SHADER_DESC desc;
        reflection->GetDesc(&desc);
        unsigned int totalSize = 0;

        for (UINT i = 0; i < desc.ConstantBuffers; i++) {
            ConstantBuffer cBuffer;
            ID3D12ShaderReflectionConstantBuffer* constantBuffer = reflection->GetConstantBufferByIndex(i);
            D3D12_SHADER_BUFFER_DESC cbDesc;
            constantBuffer->GetDesc(&cbDesc);
            cBuffer.name = cbDesc.Name;

            for (int j = 0; j < cbDesc.Variables; j++) {
                ID3D12ShaderReflectionVariable* var = constantBuffer->GetVariableByIndex(j);
                D3D12_SHADER_VARIABLE_DESC vDesc;
                var->GetDesc(&vDesc);
                ConstantBufferVariable bufferVariable;
                bufferVariable.offset = vDesc.StartOffset;
                bufferVariable.size = vDesc.Size;
                cBuffer.constantBufferData.insert({ vDesc.Name, bufferVariable });
                totalSize += bufferVariable.size;
            }
        }
        reflection->Release();

        return totalSize;
    }
};