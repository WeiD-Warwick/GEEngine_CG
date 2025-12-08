#pragma once
#include <string>
#include <map>
#include "ConstantBuffer.h"
#include "Core.h"
#include <fstream>
#include <sstream>

enum class ShaderType {
    VERTEX,
    PIXEL,
};

class Shader {
public:
    ID3DBlob* vs;
    ID3DBlob* ps;
    std::vector<ConstantBuffer> psConstantBuffers;
    std::vector<ConstantBuffer> vsConstantBuffers;
    std::map<std::string, int> textureBindPoints;

    void load(Core* core, std::string& vsPath, std::string& psPath) {
        loadBuffer(core, vsPath, ShaderType::VERTEX);
        loadBuffer(core, psPath, ShaderType::PIXEL);
    }

    // Reflection
    void buildConstantBuffer(Core* core, ID3DBlob* shader, std::vector<ConstantBuffer>& buffers) {
        ID3D12ShaderReflection* reflection;
        D3DReflect(shader->GetBufferPointer(), shader->GetBufferSize(), IID_PPV_ARGS(&reflection));
        D3D12_SHADER_DESC desc;
        reflection->GetDesc(&desc);
        for (int i = 0; i < desc.ConstantBuffers; i++) {
            ConstantBuffer buffer;
            ID3D12ShaderReflectionConstantBuffer* constantBuffer = reflection->GetConstantBufferByIndex(i);
            D3D12_SHADER_BUFFER_DESC cbDesc;
            constantBuffer->GetDesc(&cbDesc);
            buffer.name = cbDesc.Name;
            unsigned int totalSize = 0;

            for (int n = 0; n < cbDesc.Variables; n++) {
                ID3D12ShaderReflectionVariable* var = constantBuffer->GetVariableByIndex(n);
                D3D12_SHADER_VARIABLE_DESC vDesc;
                var->GetDesc(&vDesc);
                ConstantBufferVariable bufferVariable;
                bufferVariable.offset = vDesc.StartOffset;
                bufferVariable.size = vDesc.Size;
                buffer.constantBufferData.insert({ vDesc.Name, bufferVariable });
                totalSize += bufferVariable.size;
            }

            buffer.init(core, totalSize);
            buffers.push_back(buffer);
        }

        for (int i = 0; i < desc.BoundResources; i++) {
            D3D12_SHADER_INPUT_BIND_DESC bindDesc;
            reflection->GetResourceBindingDesc(i, &bindDesc);
            if (bindDesc.Type == D3D_SIT_TEXTURE)
            {
                textureBindPoints.insert({ bindDesc.Name, bindDesc.BindPoint });
            }
        }
        reflection->Release();
    }

    void updateConstantVS(std::string constantBufferName, std::string variableName, void* data) {
        updateConstant(constantBufferName, variableName, data, vsConstantBuffers);
    }

    void updateConstantPS(std::string constantBufferName, std::string variableName, void* data) {
        updateConstant(constantBufferName, variableName, data, psConstantBuffers);
    }

    void apply(Core* core) {
        for (int i = 0; i < vsConstantBuffers.size(); i++) {
            core->getCommandList()
                ->SetGraphicsRootConstantBufferView(0, vsConstantBuffers[i].getGPUAddress());

            vsConstantBuffers[i].next();
        }
        for (int i = 0; i < psConstantBuffers.size(); i++) {
            core->getCommandList()
                ->SetGraphicsRootConstantBufferView(1, psConstantBuffers[i].getGPUAddress());

            psConstantBuffers[i].next();
        }
    }

    void free() {
        vs->Release();
        ps->Release();

        for (auto cb : psConstantBuffers)
        {
            cb.free();
        }
        for (auto cb : vsConstantBuffers)
        {
            cb.free();
        }
    }

private:
    void loadBuffer(Core* core, const std::string& filePath, ShaderType type) {
        std::string src = readShader(filePath);
        switch (type) {
        case ShaderType::VERTEX:
            vs = compile(src, "VS", "vs_5_0");
            buildConstantBuffer(core, vs, vsConstantBuffers);
            break;
        case ShaderType::PIXEL:
            ps = compile(src, "PS", "ps_5_0");
            buildConstantBuffer(core, ps, psConstantBuffers);
            break;
        }
    }

    std::string readShader(const std::string& filename) {
        std::ifstream file(filename);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    ID3DBlob* compile(const std::string& src, const char* entry, const char* profile) {
        ID3DBlob* shader;
        ID3DBlob* status;

        HRESULT hr = D3DCompile(
            src.c_str(),
            strlen(src.c_str()),
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

    void updateConstant(std::string constantBufferName, std::string variableName, void* data, std::vector<ConstantBuffer>& buffers) {
        for (int i = 0; i < buffers.size(); i++)
        {
            if (buffers[i].name == constantBufferName)
            {
                buffers[i].update(variableName, data);
                return;
            }
        }
    }
};

class ShaderManager
{
public:
    std::map<std::string, Shader> shaders;

    void loadShader(Core* core, std::string& shaderName, std::string& vsPath, std::string& psPath) {
        std::map<std::string, Shader>::iterator it = shaders.find(shaderName);
        if (it != shaders.end())
        {
            return;
        }
        Shader shader;
        shader.load(core, vsPath, psPath);
        shaders.insert({ shaderName, shader });
    }

    void updateConstantVS(std::string name, std::string constantBufferName, std::string variableName, void* data)
    {
        shaders[name].updateConstantVS(constantBufferName, variableName, data);
    }

    void updateConstantPS(std::string name, std::string constantBufferName, std::string variableName, void* data)
    {
        shaders[name].updateConstantPS(constantBufferName, variableName, data);
    }

    void updateTexturePS(Core* core, const std::string& shaderName, const std::string& textureName, int heapOffset) {
        UINT bindPoint = shaders[shaderName].textureBindPoints[textureName];
        D3D12_GPU_DESCRIPTOR_HANDLE handle = core->srvHeap.gpuHandle;

        handle.ptr = handle.ptr + (UINT64)(heapOffset - bindPoint) * (UINT64)core->srvHeap.incrementSize;
        core->getCommandList()->SetGraphicsRootDescriptorTable(2, handle);
    }

    Shader* find(std::string name)
    {
        return &shaders[name];
    }
    void apply(Core* core, std::string name) {
        shaders[name].apply(core);
    }

    ~ShaderManager()
    {
        for (auto it = shaders.begin(); it != shaders.end(); )
        {
            it->second.free();
            shaders.erase(it++);
        }
    }
};
