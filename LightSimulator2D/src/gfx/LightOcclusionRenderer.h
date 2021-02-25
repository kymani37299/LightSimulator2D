#pragma once

#include <vector>
#include <queue>
#include <map>

#include "common.h"

#include "shaders/common.h"

class Entity;

class ShaderStorageBuffer;
class ShaderInput;
class UniformBuffer;
class ComputeShader;
class Shader;

class Framebuffer;

class LightOcclusionRenderer
{
public:
	LightOcclusionRenderer();
	~LightOcclusionRenderer();
	
	void Update(float dt) { m_TimeSinceLastDraw += dt; }

	void CompileShaders();

	void OnOccluderAdded(Entity* e);
	void OnOccluderRemoved(Entity* e);

	void RenderOcclusion();

	Framebuffer* GetOcclusionMaskFB() { return m_OcclusionMaskFB; }

private:
	void SetupLineSegments();
	void SetupRayQuery();
	unsigned SetupOcclusionMeshInput();

	using OcclusionMesh = std::vector<Vec2>;
	void PopulateOcclusionMesh(OcclusionMesh& mesh);
	
	void LightOcclusion();
	void TriangulateMeshes();
	void RenderOcclusionMask();
	void MergeMasks();

	Framebuffer* GetCurrentOcclusionMask() { return m_OcclusionMaskPP ? m_OcclusionMaskFB1 : m_OcclusionMaskFB2; }
	Framebuffer* GetOtherOcclusionMask() { return m_OcclusionMaskPP ? m_OcclusionMaskFB2 : m_OcclusionMaskFB1; }

private:

	static constexpr unsigned NUM_ANGLED_RAYS = 30;
	static constexpr unsigned NUM_LIGHT_SAMPLES = 6;
	static constexpr float DRAW_INTERVAL = 50.0f;

	Vec2 m_LightSource;
	float m_LightRadius = 0.03f;
	float m_MaskStrength = 1.0f / NUM_LIGHT_SAMPLES * 1.3f;

	std::map<Entity*, OcclusionMesh> m_OcclusionMeshPool;

	unsigned m_OcclusionLineCount = 0;
	ShaderInput* m_OcclusionMesh = nullptr;

	ShaderStorageBuffer* m_OcclusionMeshOutput;

	float m_TimeSinceLastDraw = 0.0;

	bool m_OcclusionMaskPP = false;
	Framebuffer* m_OcclusionMaskFB1;
	Framebuffer* m_OcclusionMaskFB2;
	Framebuffer* m_OcclusionMaskFB;

	struct RayAngleComparator
	{
		bool operator () (const Vec2& l, const Vec2& r)
		{
			return glm::atan(l.y, l.x) < glm::atan(r.y, r.x);
		}
	};
	using RayQuery = std::priority_queue<Vec2, std::vector<Vec2>, RayAngleComparator>;
	RayQuery m_RayQuery;

	ComputeShader* m_OcclusionMeshGenShader = nullptr;
	Shader* m_ShadowmapShader = nullptr;
	Shader* m_MergeShader = nullptr;

	unsigned m_RayCount = 0;

	ShaderStorageBuffer* m_IntersectionBuffer;
	ShaderStorageBuffer* m_TriangledIntersecitonsBuffer;
	UniformBuffer* m_OcclusionLines;
	UniformBuffer* m_RayQueryBuffer;

	ComputeShader* m_OcclusionShader = nullptr;
	ComputeShader* m_TriangulationShader = nullptr;
};