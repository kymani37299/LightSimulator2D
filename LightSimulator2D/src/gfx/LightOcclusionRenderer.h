#pragma once

#include <vector>
#include <map>

#include "common.h"

#include "shaders/common.h"

class Entity;

class CulledScene;

class ShaderStorageBuffer;
class ShaderInput;
class UniformBuffer;
class Shader;

class Framebuffer;

struct OcclusionQuery
{
	Vec2 position;
	Vec3 color;
	float radius;
	float strength;
};

class LightOcclusionRenderer
{
public:
	LightOcclusionRenderer();
	~LightOcclusionRenderer();
	
	void Update(float dt) { m_TimeSinceLastDraw += dt; }

	void CompileShaders();

	void OnOccluderAdded(Entity* e);
	void OnOccluderRemoved(Entity* e);

	void RenderOcclusion(CulledScene& scene);

	Framebuffer* GetOcclusionMaskFB() { return m_OcclusionMaskFinal; }

private:
	void SetupLineSegments(CulledScene& scene);
	void SetupRayQuery();
	unsigned SetupOcclusionMeshInput();

	using OcclusionMesh = std::vector<Vec2>;
	void PopulateOcclusionMesh(OcclusionMesh& mesh, int meshSize);
	
	void LightOcclusion(CulledScene& scene);
	void TriangulateMeshes();
	void RenderOcclusionMask(CulledScene& scene);
	void MergeMasks();
	void BlurMask();

#ifdef INTERVAL_OCCLUSION
	Framebuffer* GetCurrentOcclusionMask() { return m_OcclusionMaskPP ? m_OcclusionMaskFB1 : m_OcclusionMaskFB2; }
	Framebuffer* GetOtherOcclusionMask() { return m_OcclusionMaskPP ? m_OcclusionMaskFB2 : m_OcclusionMaskFB1; }
#else
	Framebuffer* GetCurrentOcclusionMask() { return m_OcclusionMaskFB; }
	Framebuffer* GetOtherOcclusionMask() { return m_OcclusionMaskFB; }
#endif

private:

	static constexpr unsigned NUM_ANGLED_RAYS = 30;
	static constexpr unsigned NUM_LIGHT_SAMPLES = 6;

#ifdef INTERVAL_OCCLUSION
	static constexpr float DRAW_INTERVAL = 50.0f;
#else
	static constexpr float DRAW_INTERVAL = 0.0f;
#endif

	OcclusionQuery m_CurrentQuery;

	std::map<Entity*, OcclusionMesh> m_OcclusionMeshPool;

	unsigned m_OcclusionLineCount = 0;
	ShaderInput* m_OcclusionMesh = nullptr;

	ShaderStorageBuffer* m_OcclusionMeshOutput;

	float m_TimeSinceLastDraw = 0.0;

#ifdef INTERVAL_OCCLUSION
	bool m_OcclusionMaskPP = false;
	Framebuffer* m_OcclusionMaskFB1;
	Framebuffer* m_OcclusionMaskFB2;
#endif

	Framebuffer* m_OcclusionMaskFB;
	Framebuffer* m_OcclusionMaskFinal;

	std::vector<Vec2> m_RayQuery;

	Shader* m_OcclusionMeshGenShader = nullptr;
	Shader* m_ShadowmapShader = nullptr;
	Shader* m_MergeShader = nullptr;
	Shader* m_BlurShader = nullptr;

	unsigned m_RayCount = 0;

	ShaderStorageBuffer* m_IntersectionBuffer;
	ShaderStorageBuffer* m_TriangledIntersecitonsBuffer;
	UniformBuffer* m_OcclusionLines;
	UniformBuffer* m_RayQueryBuffer;

	Shader* m_OcclusionShader = nullptr;
	Shader* m_TriangulationShader = nullptr;
};