#pragma once
#include <string>
#include "../Foundation/Mesh.h"
#include "../Foundation/PSOManager.h"
#include "../Foundation/ShaderManager.h"
#include "../Foundation/VertexLayoutCache.h"

class Plane {
public:
	Mesh mesh;
	std::string shaderName;

	void init(Core* core, PSOManager* psoManager, ShaderManager* shaderManager) {
		std::vector<STATIC_VERTEX> vertices;
		vertices.push_back(addVertex(Vec3(-1, 0, -1), Vec3(0, 1, 0), 0, 0));
		vertices.push_back(addVertex(Vec3(1, 0, -1), Vec3(0, 1, 0), 1, 0));
		vertices.push_back(addVertex(Vec3(-1, 0, 1), Vec3(0, 1, 0), 0, 1));
		vertices.push_back(addVertex(Vec3(1, 0, 1), Vec3(0, 1, 0), 1, 1));
		std::vector<unsigned int> indices;
		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(2);


		indices.push_back(1);
		indices.push_back(3);
		indices.push_back(2);
		mesh.init(core, vertices, indices);

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

	void draw(Core* core, PSOManager* psos, ShaderManager* shaders, float time) {
		Matrix W;
		Matrix vp;
		Matrix p = Matrix::perspectiveLH(0.01f, 10000.0f, 1024.0f / 1024.0f, 60.0f);
		Vec3 from = Vec3(11 * cos(time), 5, 11 * sinf(time));
		Matrix v = Matrix::lookAtLH(from, Vec3(0, 0, 0), Vec3(0, 1, 0));
		vp = p * v;
		shaders->updateConstantVS(shaderName, "staticMeshBuffer", "VP", &vp);
		shaders->updateConstantVS(shaderName, "staticMeshBuffer", "W", &W);
		shaders->apply(core, shaderName);
		psos->bind(core, "StaticModelUntexturedPSO");
		mesh.draw(core);
	}
};
