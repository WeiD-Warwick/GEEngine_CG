#pragma once
#pragma once
#include <d3d12.h>
#include <string>
#include <unordered_map>
#include "ConstantBuffer.h"

class ShaderManager
{
public:
    struct ShaderRecord
    {
        ConstantBuffer vsCB;   // VS constant buffer
        std::string cbName;    // staticMeshBuffer
    };

    std::unordered_map<std::string, ShaderRecord> shaders;

    void registerVSConstantBuffer(std::string shaderName, ConstantBuffer cb, std::string cbName)
    {
        ShaderRecord r;
        r.vsCB = cb;
        r.cbName = cbName;
        shaders[shaderName] = r;
    }

    void updateConstantVS(
        std::string shaderName,
        std::string cbName,
        std::string varName,
        void* data)
    {
        ShaderRecord& rec = shaders[shaderName];
        rec.vsCB.update(varName, data);
    }

    void bindVS(Core* core, std::string shaderName)
    {
        ShaderRecord& rec = shaders[shaderName];
        core->getCommandList()->SetGraphicsRootConstantBufferView(
            0, rec.vsCB.getGPUAddress());
        rec.vsCB.next();
    }
};
