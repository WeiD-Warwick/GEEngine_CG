#pragma once
#include <string>
#include "../Foundation/Mesh.h"
#include "../Foundation/PSOManager.h"
#include "../Foundation/ShaderManager.h"
#include "../../Third_Party/GEMLoader.h"
#include "../Foundation/VertexLayoutCache.h"

struct InstanceData {
	Matrix data;
};


class Tree {
	std::vector<Mesh*> meshes;
	std::string shaderName;

	// ---------- Instancing ----------
	ID3D12Resource* instanceBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW instanceVBV = {};
	UINT instanceCount = 0;
	Matrix baseW;



public:
	void init(Core* core, PSOManager* psoManager, ShaderManager* shaderManager, std::string filename) {

		GEMLoader::GEMModelLoader loader;
		std::vector<GEMLoader::GEMMesh> gemmeshes;
		loader.load(filename, gemmeshes);
		for (int i = 0; i < gemmeshes.size(); i++) {
			Mesh* mesh = new Mesh();
			std::vector<STATIC_VERTEX> vertices;
			for (int j = 0; j < gemmeshes[i].verticesStatic.size(); j++)
			{
				STATIC_VERTEX v;
				memcpy(&v, &gemmeshes[i].verticesStatic[j], sizeof(STATIC_VERTEX));
				vertices.push_back(v);
			}
			mesh->init(core, vertices, gemmeshes[i].indices);
			meshes.push_back(mesh);
		}

		shaderName = "StaticModelInstanced";
		std::string vsPath = "Src/Shaders/VS_Instanced.hlsl";
		std::string psPath = "Src/Shaders/PS.hlsl";

		shaderManager->loadShader(core, shaderName, vsPath, psPath);
		psoManager->createPSO(
			core,
			"StaticInstancedPSO",
			shaderManager->find(shaderName)->vs,
			shaderManager->find(shaderName)->ps,
			VertexLayoutCache::getStaticInstancedLayout());

		baseW = Matrix::scaling(Vec3(0.01f, 0.01f, 0.01f));
		createTreeInstances(core);

	}

	void draw(Core* core, PSOManager* psoManager, ShaderManager* shaderManager, float time) {

		Matrix vp;
		Matrix p = Matrix::perspectiveLH(0.01f, 10000.0f, 1024.0f / 1024.0f, 60.0f);
		Vec3 from = Vec3(50 * cos(time), 50, 50 * sinf(time));
		Matrix v = Matrix::lookAtLH(from, Vec3(0, 0, 0), Vec3(0, 1, 0));
		vp = p * v;

		auto* commandList = core->getCommandList();

		shaderManager->updateConstantVS(shaderName, "staticMeshBuffer", "VP", &vp);
		shaderManager->apply(core, shaderName);
		psoManager->bind(core, "StaticInstancedPSO");

		for (Mesh* mesh : meshes)
		{
			D3D12_VERTEX_BUFFER_VIEW vbvs[2] = { mesh->getVBV(), instanceVBV };

			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			commandList->IASetVertexBuffers(0, 2, vbvs);
			commandList->IASetIndexBuffer(&mesh->getIBV());
			commandList->DrawIndexedInstanced(mesh->getIndexCount(), instanceCount, 0, 0, 0);
		}

	}

	void createTreeInstances(Core* core)
	{
		instanceCount = 3;
		InstanceData instances[3];

		Matrix W0 = Matrix::Identity().scaling(Vec3(0.01, 0.01, 0.01));

		Matrix W1 = Matrix::translation(Vec3(3.0f, 0.0f, 0.0f))
			* Matrix::scaling(Vec3(0.01f, 0.01f, 0.01f));

		Matrix W2 = Matrix::translation(Vec3(-3.0f, 0.0f, 0.0f))
			* Matrix::scaling(Vec3(0.01f, 0.01f, 0.01f));


		Matrix Ws[3] = { W0, W1, W2 };

		for (int i = 0; i < 3; i++)
		{
			instances[i].data = Ws[i];
		}

		D3D12_HEAP_PROPERTIES heap = {};
		heap.Type = D3D12_HEAP_TYPE_UPLOAD;

		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Width = sizeof(instances);
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		core->device->CreateCommittedResource(
			&heap,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&instanceBuffer)
		);

		void* mapped = nullptr;
		instanceBuffer->Map(0, nullptr, &mapped);
		memcpy(mapped, instances, sizeof(instances));
		instanceBuffer->Unmap(0, nullptr);

		instanceVBV.BufferLocation = instanceBuffer->GetGPUVirtualAddress();
		instanceVBV.StrideInBytes = sizeof(InstanceData);
		instanceVBV.SizeInBytes = sizeof(instances);
	}



	~Tree() {
		for (Mesh* m : meshes) delete m;
		meshes.clear();

		if (instanceBuffer) { instanceBuffer->Release(); instanceBuffer = nullptr; }
	}
};