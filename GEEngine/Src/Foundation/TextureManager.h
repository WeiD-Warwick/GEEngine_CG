#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include "../../Third_Party/stb_image.h"
#include <d3d12.h>
#include "Core.h"
#include <string>
#include <map>

class Texture {

public:
	ID3D12Resource* tex = nullptr;
	int heapOffset = -1;
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	void init(Core* core, const std::string& filename) {
		int width = 0;
		int height = 0;
		int channels = 0;
		unsigned char* texels = stbi_load(filename.c_str(), &width, &height, &channels, 0);
		if (!texels)
		{
			OutputDebugStringA(("Failed to load texture: " + filename + "\n").c_str());
			return;
		}
		if (channels == 3) {
			channels = 4;
			unsigned char* texelsWithAlpha = new unsigned char[width * height * channels];
			for (int i = 0; i < (width * height); i++) {
				texelsWithAlpha[i * 4] = texels[i * 3];
				texelsWithAlpha[(i * 4) + 1] = texels[(i * 3) + 1];
				texelsWithAlpha[(i * 4) + 2] = texels[(i * 3) + 2];
				texelsWithAlpha[(i * 4) + 3] = 255;
			}
			// Initialize texture using width, height, channels, and texelsWithAlpha
			delete[] texelsWithAlpha;
		}
		else {
			// Initialize texture using width, height, channels, and texels
		}

		// Create GPU Texture
		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

		D3D12_RESOURCE_DESC textureDesc = {};
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.DepthOrArraySize = 1;
		textureDesc.MipLevels = 1;
		textureDesc.Format = format;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

		HRESULT hr = core->device->CreateCommittedResource(
			&heapProps, D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&tex)
		);

		if (FAILED(hr)) {
			OutputDebugStringA("CreateCommittedResource failed in Texture\n");
			return;
		}

		// 

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
		UINT64 totalBytes = 0;

		core->device->GetCopyableFootprints(
			&textureDesc,
			0, 1, 0,
			&footprint,
			nullptr, nullptr, &totalBytes
		);

		UINT rowPitch = footprint.Footprint.RowPitch;

		// uploadData
		unsigned char* uploadData = new unsigned char[rowPitch * height];

		for (UINT y = 0; y < height; y++)
		{
			memcpy(uploadData + y * rowPitch,
				texels + y * width * channels,
				width * channels);
		}

		core->uploadResource(
			tex,
			uploadData,
			rowPitch * height,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&footprint
		);

		delete[] uploadData;

		stbi_image_free(texels);

		D3D12_CPU_DESCRIPTOR_HANDLE h = core->srvHeap.getNextCPUHandle();

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = 1;

		core->device->CreateShaderResourceView(tex, &srvDesc, h);

		heapOffset = core->srvHeap.used - 1;

	}
};

class TextureManager
{
public:
	std::map<std::string, Texture*> textures;

	Texture* loadTexture(Core* core, const std::string& name, const std::string& file) {
		auto it = textures.find(name);
		if (it != textures.end())
			return it->second;

		Texture* t = new Texture();
		t->init(core, file);

		textures[name] = t;
		return t;
	}

	int find(const std::string& name) {
		auto it = textures.find(name);
		if (it == textures.end()) return -1;
		return it->second->heapOffset;
	}
};

