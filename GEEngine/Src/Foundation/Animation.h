#pragma once
#include "CoreMath.h"
#include <d3d12.h>
#include <vector>
#include <string>
#include <map>
#include <algorithm>

struct Bone {
	std::string name;
	Matrix offset;
	int parentIndex;
};

struct Skeleton {
	std::vector<Bone> bones;
	Matrix globalInverse;

	int findBone(std::string name) {
		for (int i = 0; i < bones.size(); i++) {
			if (bones[i].name == name) {
				return i;
			}
		}
		return -1;
	}
};

struct AnimationFrame {
	std::vector<Vec3> positions;
	std::vector<Quaternion> rotations;
	std::vector<Vec3> scales;
};

class AnimationSequence {
public:
	std::vector<AnimationFrame> frames;
	float ticksPerSecond;

	Vec3 interpolate(Vec3 p1, Vec3 p2, float t) {
		return ((p1 * (1.0f - t)) + (p2 * t));
	}

	Quaternion interpolate(Quaternion q1, Quaternion q2, float t) {
		return Quaternion::slerp(q1, q2, t);
	}

	float duration() {
		return ((float)frames.size() / ticksPerSecond);
	}

	void calcFrame(float t, int& frame, float& interpolationFact) {
		interpolationFact = t * ticksPerSecond;
		frame = (int)floorf(interpolationFact);
		interpolationFact = interpolationFact - (float)frame;
		frame = std::min<int>(frame, frames.size() - 1);
	}

	int nextFrame(int frame) {
		return std::min<int>(frame + 1, frames.size() - 1);
	}

	Matrix interpolateBoneToGlobal(Matrix* matrices, int baseFrame, float interpolationFact,
		Skeleton* skeleton, int boneIndex) {

		Matrix scale = Matrix::scaling(
			interpolate(frames[baseFrame].scales[boneIndex],
						frames[nextFrame(baseFrame)].scales[boneIndex],
						interpolationFact));

		Matrix rotation = 
			interpolate(frames[baseFrame].rotations[boneIndex],
						frames[nextFrame(baseFrame)].rotations[boneIndex], 
						interpolationFact).toMatrix();

		Matrix translation = Matrix::translation(
			interpolate(frames[baseFrame].positions[boneIndex],
						frames[nextFrame(baseFrame)].positions[boneIndex],
						interpolationFact));

		Matrix local = translation * rotation * scale;

		int parentIndex = skeleton->bones[boneIndex].parentIndex;
		if (parentIndex > -1) {
			Matrix global = matrices[parentIndex] * local;
			return global;
		}
		return local;
	}
};


class Animation
{
public:
	std::map<std::string, AnimationSequence> animations;
	Skeleton skeleton;

	int bonesSize() {
		return skeleton.bones.size();
	}

	void calcFrame(std::string name, float t, int& frame, float& interpolationFact) {
		animations[name].calcFrame(t, frame, interpolationFact);
	}

	Matrix interpolateBoneToGlobal(std::string name, Matrix* matrices, int baseFrame, float interpolationFact, int boneIndex) {
		return animations[name].interpolateBoneToGlobal(matrices, baseFrame, interpolationFact, &skeleton, boneIndex);
	}

	void calcTransformsFromGlobal(Matrix* globalMatrices, Matrix* outMatrices, Matrix coordTransform) {
		for (int i = 0; i < bonesSize(); i++) {
			outMatrices[i] =
				globalMatrices[i] *
				skeleton.bones[i].offset *
				skeleton.globalInverse *
				coordTransform;
		}
	}

	void calcTransforms(Matrix* matrices, Matrix coordTransform) {
		calcTransformsFromGlobal(matrices, matrices, coordTransform);
	}

	bool hasAnimation(std::string name) {
		if (animations.find(name) == animations.end())
		{
			return false;
		}
		return true;
	}
};

class AnimationInstance {
private:
	Matrix globalMatrices[256];

	std::vector<int> boneOrder;

public:
	Animation* animation;
	std::string currentAnimation;
	float t;
	Matrix matrices[256];
	Matrix coordTransform;

	void resetAnimationTime() {
		t = 0;
	}

	void init(Animation* _animation, int fromYZX) {
		animation = _animation;
		if (fromYZX == 1)
		{
			memset(coordTransform.a, 0, 16 * sizeof(float));
			coordTransform.a[0][0] = 1.0f;
			coordTransform.a[2][1] = 1.0f;
			coordTransform.a[1][2] = -1.0f;
			coordTransform.a[3][3] = 1.0f;
		}
		buildBoneOrder();
	}

	bool animationFinished() {
		if (t > animation->animations[currentAnimation].duration())
		{
			return true;
		}
		return false;
	}

	void update(std::string name, float dt)
	{
		if (name == currentAnimation)
		{
			t += dt;
		}
		else
		{
			currentAnimation = name;
			t = 0;
		}
		if (animationFinished() == true)
		{
			return;
		}
		int frame = 0;
		float interpolationFact = 0;
		animation->calcFrame(name, t, frame, interpolationFact);
		for (int i = 0; i < animation->bonesSize(); i++)
		{
			matrices[i] = animation->interpolateBoneToGlobal(name, matrices, frame, interpolationFact, i);
		}

		for (int idx : boneOrder) {
			globalMatrices[idx] = animation->interpolateBoneToGlobal(
				name,
				globalMatrices,
				frame,
				interpolationFact,
				idx);
		}

		animation->calcTransformsFromGlobal(globalMatrices, matrices, coordTransform);
	}


	Matrix findWorldMatrix(std::string boneName) {
		if (!animation) return Matrix::Identity();

		int boneID = animation->skeleton.findBone(boneName);
		if (boneID < 0) {
			return Matrix::Identity();
		}

		if (currentAnimation.empty()) {
			currentAnimation = animation->animations.begin()->first;
			t = 0.0f;
		}

		int frame = 0;
		float interpolationFact = 0.0f;
		animation->calcFrame(currentAnimation, t, frame, interpolationFact);

		for (int idx : boneOrder) {
			globalMatrices[idx] = animation->interpolateBoneToGlobal(
				currentAnimation,
				globalMatrices,
				frame,
				interpolationFact,
				idx);
		}

		return globalMatrices[boneID] * coordTransform;
	}

	private:
		void buildBoneOrder() {
			boneOrder.clear();
			if (!animation) return;

			int n = animation->bonesSize();
			if (n <= 0) return;

			for (int i = 0; i < n; ++i) {
				if (animation->skeleton.bones[i].parentIndex == -1) {
					dfsAddBone(i, n);
				}
			}
		}

		void dfsAddBone(int boneIndex, int boneCount) {
			boneOrder.push_back(boneIndex);
			for (int i = 0; i < boneCount; ++i) {
				if (animation->skeleton.bones[i].parentIndex == boneIndex) {
					dfsAddBone(i, boneCount);
				}
			}
		}
};