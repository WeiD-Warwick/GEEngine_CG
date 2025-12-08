#pragma once
#include <string>
#include "../Foundation/Mesh.h"
#include "../Foundation/PSOManager.h"
#include "../Foundation/ShaderManager.h"
#include "../../Third_Party/GEMLoader.h"
#include "../Foundation/VertexLayoutCache.h"
#include "../Foundation/Animation.h"
#include "../Foundation/TextureManager.h"

class AnimatedModel {
	std::vector<Mesh *> meshes;
	Animation animation;
	std::string shaderName = "Animated";
	std::string psoName = "AnimatedPSO";
	std::string constBufferName = "animatedMeshBuffer";

	AnimationInstance instance;

	std::vector<std::string> textureFilenames;

public:
	void load(Core* core, PSOManager* psoManager, ShaderManager* shaderManager, TextureManager* textureManager, std::string filename) {

		GEMLoader::GEMModelLoader loader;
		std::vector<GEMLoader::GEMMesh> gemmeshes;
		GEMLoader::GEMAnimation gemanimation;

		// load data
		loader.load(filename, gemmeshes, gemanimation);
		for (int i = 0; i < gemmeshes.size(); i++) {
			Mesh* mesh = new Mesh();
			std::vector<ANIMATED_VERTEX> vertices;

			for (int j = 0; j < gemmeshes[i].verticesAnimated.size(); j++) {
				ANIMATED_VERTEX v;
				memcpy(&v, &gemmeshes[i].verticesAnimated[j], sizeof(ANIMATED_VERTEX));
				vertices.push_back(v);
			}
			std::string texName = gemmeshes[i].material.find("albedo").getValue();
			std::string fullPath = "Src/Assets/" + texName;
			textureManager->loadTexture(core, texName, fullPath);
			textureFilenames.push_back(texName);
			mesh->init(core, vertices, gemmeshes[i].indices);
			meshes.push_back(mesh);
		}

		// load globalInverse
		memcpy(&animation.skeleton.globalInverse, &gemanimation.globalInverse, 16 * sizeof(float));

		// load bones
		for (int i = 0; i < gemanimation.bones.size(); i++) {
			Bone bone;
			bone.name = gemanimation.bones[i].name;
			memcpy(&bone.offset, &gemanimation.bones[i].offset, 16 * sizeof(float));
			bone.parentIndex = gemanimation.bones[i].parentIndex;
			animation.skeleton.bones.push_back(bone);
		}

		// load animation data
		for (int i = 0; i < gemanimation.animations.size(); i++) {
			std::string name = gemanimation.animations[i].name;
			AnimationSequence aseq;
			aseq.ticksPerSecond = gemanimation.animations[i].ticksPerSecond;
			for (int n = 0; n < gemanimation.animations[i].frames.size(); n++) {
				AnimationFrame frame;
				for (int index = 0; index < gemanimation.animations[i].frames[n].positions.size(); index++) {
					Vec3 p;
					Quaternion q;
					Vec3 s;
					memcpy(&p, &gemanimation.animations[i].frames[n].positions[index], sizeof(Vec3));
					frame.positions.push_back(p);
					memcpy(&q, &gemanimation.animations[i].frames[n].rotations[index], sizeof(Quaternion));
					frame.rotations.push_back(q);
					memcpy(&s, &gemanimation.animations[i].frames[n].scales[index], sizeof(Vec3));
					frame.scales.push_back(s);
				}
				aseq.frames.push_back(frame);
			}
			animation.animations.insert({ name, aseq });
		}

		// Load Shaders
		std::string vsPath = "Src/Shaders/VS_Animation.hlsl";
		std::string psPath = "Src/Shaders/PS_Animation.hlsl";
		shaderManager->loadShader(core, shaderName, vsPath, psPath);

		// Create PSO for animations
		psoManager->createPSO(
			core,
			psoName,
			shaderManager->find(shaderName)->vs,
			shaderManager->find(shaderName)->ps,
			VertexLayoutCache::getAnimatedLayout());

		instance.init(&animation, 0);
	}

	void update(float dt) {
		instance.update("run", dt);
		if (instance.animationFinished() == true) {
			instance.resetAnimationTime();
		}
	}

	void draw(Core* core, PSOManager* psoManager, ShaderManager* shaderManager, TextureManager* textureManager, float time, Matrix w) {
		Matrix vp;
		Matrix p = Matrix::perspectiveLH(0.01f, 10000.0f, 1024.0f / 1024.0f, 60.0f);
		Vec3 from = Vec3(11 * cos(time), 5, 11 * sinf(time));
		Matrix v = Matrix::lookAtLH(from, Vec3(0, 0, 0), Vec3(0, 1, 0));
		vp = p * v;
		shaderManager->updateConstantVS(shaderName, constBufferName, "W", &w);
		shaderManager->updateConstantVS(shaderName, constBufferName, "VP", &vp);
		shaderManager->updateConstantVS(shaderName, constBufferName, "bones", instance.matrices);
		shaderManager->apply(core, shaderName);

		psoManager->bind(core, psoName);

		for (int i = 0; i < meshes.size(); i++) {
			shaderManager->updateTexturePS(core, shaderName, "tex", textureManager->find(textureFilenames[i]));
			meshes[i]->draw(core);
		}
	}

};