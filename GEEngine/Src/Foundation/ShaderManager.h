#pragma once
#include <string>
#include <map>
#include "ConstantBuffer.h"
#include "Core.h"
#include <fstream>
#include <sstream>

struct ShaderInfo {
    ID3DBlob* blobVS = nullptr;
    ID3DBlob* blobPS = nullptr;

    std::map<std::string, ConstantBuffer*> vsCBs;
    std::map<std::string, ConstantBuffer*> psCBs;
};

class ShaderManager
{
public:
    std::map<std::string, ShaderInfo> shaders;

    static ID3DBlob* loadAndCompile(const std::string& filename, const char* entry, const char* profile) {
        std::string src = ReadShader(filename);
        return Compile(src, entry, profile);
    }

    void addConstantBufferVS(const std::string& shaderName,
        const std::string& cbName,
        ConstantBuffer* cb) {
        shaders[shaderName].vsCBs[cbName] = cb;
    }

    void updateConstantVS(Core* core, 
                          const std::string& shaderName,
                          const std::string& cbName,
                          const std::string& varName,
                          const void* data) {
        ConstantBuffer* cb = shaders[shaderName].vsCBs[cbName];
        cb->update(varName, data, core->frameIndex());
    }


private:

    static std::string ReadShader(const std::string& filename) {
        std::ifstream file(filename);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    static ID3DBlob* Compile(const std::string& src, const char* entry, const char* profile) {
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
};
