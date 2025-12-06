#pragma once
#include <string>
#include "../Foundation/Mesh.h"
#include "../Foundation/PSOManager.h"
#include "../Foundation/ShaderManager.h"
#include "../../Third_Party/GEMLoader.h"
#include "../Foundation/VertexLayoutCache.h"

class Tree {
	std::vector<Mesh*> meshes;
	std::string shaderName;

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

		shaderName = "StaticModelUntextured";
		std::string vsPath = "Src/Shaders/VS.hlsl";
		std::string psPath = "Src/Shaders/PS.hlsl";

		shaderManager->loadShader(core, shaderName, vsPath, psPath);
		psoManager->createPSO(
			core,
			"StaticModelUntexturedPSO",
			shaderManager->find(shaderName)->vs,
			shaderManager->find(shaderName)->ps,
			VertexLayoutCache::getStaticLayout());
	}

	void draw(Core* core, PSOManager* psoManager, ShaderManager* shaderManager, float time, Matrix w) {

		Matrix vp;
		Matrix p = Matrix::perspectiveLH(0.01f, 10000.0f, 1024.0f / 1024.0f, 60.0f);
		Vec3 from = Vec3(11 * cos(time), 5, 11 * sinf(time));
		Matrix v = Matrix::lookAtLH(from, Vec3(0, 0, 0), Vec3(0, 1, 0));
		vp = p * v;

		shaderManager->updateConstantVS(shaderName, "staticMeshBuffer", "VP", &vp);
		shaderManager->updateConstantVS(shaderName, "staticMeshBuffer", "W", &w);
		shaderManager->apply(core, shaderName);
		psoManager->bind(core, "StaticModelUntexturedPSO");

		for (int i = 0; i < meshes.size(); i++) {
			meshes[i]->draw(core);
		}
	}

	~Tree() {
		for (Mesh* m : meshes) {
			delete m;
		}
		meshes.clear();
	}
};