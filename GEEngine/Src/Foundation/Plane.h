#pragma once
#include "Mesh.h"
#include <string>
#include "PSOManager.h"
#include "ShaderManager.h"

class Plane {

public:
	Mesh mesh;
	std::string shaderName;

	void init(Core* core, PSOManager* psoManager, ShaderManager* shaderManager)
	{
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
			shaderManager->find("StaticModelUntextured")->vs, 
			shaderManager->find("StaticModelUntextured")->ps,
			VertexLayoutCache::getStaticLayout());
	}

	void draw(Core* core, PSOManager* psos, ShaderManager* shaders)
	{
		Matrix planeWorld;
		shaders->updateConstantVS("StaticModelUntextured", "staticMeshBuffer", "W", &planeWorld);
		shaders->apply(core, shaderName);
		psos->bind(core, "StaticModelUntexturedPSO");
		mesh.draw(core);
	}
};
