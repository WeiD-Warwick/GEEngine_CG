#pragma once
#include <string>
#include <map>
#include "ConstantBuffer.h"
#include "Core.h"

struct ShaderConstantBuffers
{
    std::map<std::string, ConstantBuffer*> vsCBs;
    std::map<std::string, ConstantBuffer*> psCBs;
};

class ShaderManager
{
public:
    std::map<std::string, ShaderConstantBuffers> shaders;

    void addVSConstantBuffer(const std::string& shaderName,
        const std::string& cbName,
        ConstantBuffer* cb) {
        shaders[shaderName].vsCBs[cbName] = cb;
    }

    void addPSConstantBuffer(const std::string& shaderName,
        const std::string& cbName,
        ConstantBuffer* cb)
    {
        shaders[shaderName].psCBs[cbName] = cb;
    }

    void updateConstantVS(const std::string& shaderName,
        const std::string& cbName,
        const std::string& varName,
        void* data)
    {
        ConstantBuffer* cb = shaders[shaderName].vsCBs[cbName];
        cb->update(varName, data);
    }

    void updateConstantPS(const std::string& shaderName,
        const std::string& cbName,
        const std::string& varName,
        void* data)
    {
        ConstantBuffer* cb = shaders[shaderName].psCBs[cbName];
        cb->update(varName, data);
    }

    void applyVS(Core* core, const std::string& shaderName)
    {
        int slot = 0;
        for (auto& it : shaders[shaderName].vsCBs)
        {
            ConstantBuffer* cb = it.second;
            core->getCommandList()->SetGraphicsRootConstantBufferView(
                slot, cb->getGPUAddress(core->frameIndex())
            );
            cb->next();
            slot++;
        }
    }

    void applyPS(Core* core, const std::string& shaderName)
    {
        int slot = 0;
        for (auto& it : shaders[shaderName].psCBs)
        {
            ConstantBuffer* cb = it.second;
            core->getCommandList()->SetGraphicsRootConstantBufferView(
                slot, cb->getGPUAddress(core->frameIndex())
            );
            cb->next();
            slot++;
        }
    }
};
