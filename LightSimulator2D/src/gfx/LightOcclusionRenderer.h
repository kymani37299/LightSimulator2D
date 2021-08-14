#pragma once

#include <vector>
#include <map>

#include "common.h"

#include "shaders/common.h"

class Entity;
class EntityInstance;

class CulledScene;

class ShaderStorageBuffer;
class ShaderInput;
class UniformBuffer;
class Shader;

class Framebuffer;

struct OcclusionQuery
{
	Vec2 position = VEC2_ZERO;
	Vec3 color = VEC3_ZERO;
	float radius = 0.0f;
	float strength = 0.0f;
};

enum OcclusionDebugOption
{
	OcclusionDebug_Intersections = 1,
	OcclusionDebug_Rays = 2,
	OcclusionDebug_Mesh = 4,
	OcclusionDebug_DisableAngledRays = 8,
	OcclusionDebug_SimpleLightMask = 16
	// 32
};

class LightOcclusionRenderer
{
public:
	LightOcclusionRenderer();
	~LightOcclusionRenderer();

	void CompileShaders();

	void OnOccluderAdded(Entity* e);
	void OnOccluderRemoved(Entity* e);

	void RenderOcclusion(CulledScene& scene);

	inline Framebuffer* GetOcclusionMaskFB() const { return m_OcclusionMaskFinal; }
	inline void SetDebugOptions(unsigned int debugOptions) { m_DebugOptions = debugOptions; }

private:
	void SetupBuffers(CulledScene& scene);
	std::map<EntityInstance*,unsigned> DivideSamples(CulledScene& scene);

	using OcclusionMesh = std::vector<Vec2>;
	void PopulateOcclusionMesh(OcclusionMesh& mesh, int meshSize);
	
	void LightOcclusion(CulledScene& scene);
	void SortIntersections();
	void RenderOcclusionMask(CulledScene& scene);
	void BlurMask();
	void DrawDebug(CulledScene& scene);

private:

	static constexpr unsigned NUM_ANGLED_RAYS = 30;

	static constexpr unsigned OPTIMAL_LIGHT_SAMPLES = 6;
	static constexpr unsigned MAX_LIGHT_SAMPLES = 12;

	OcclusionQuery m_CurrentQuery;
	std::map<Entity*, OcclusionMesh> m_OcclusionMeshPool;

	unsigned m_RayCount = 0;
	std::vector<Vec4> m_RayQuery;

	unsigned m_OcclusionLineCount = 0;
	ShaderInput* m_OcclusionMesh = nullptr;

	Shader* m_OcclusionMeshGenShader = nullptr;
	Shader* m_ShadowmapShader = nullptr;
	Shader* m_BlurShader = nullptr;
	Shader* m_OcclusionShader = nullptr;
	Shader* m_TriangulationShader = nullptr;

	ShaderStorageBuffer* m_IntersectionBuffer;
	ShaderStorageBuffer* m_OcclusionMeshOutput;

	UniformBuffer* m_OcclusionLines;
	UniformBuffer* m_RayQueryBuffer;

	Framebuffer* m_OcclusionMaskFB;
	Framebuffer* m_OcclusionMaskFinal;

	unsigned int m_DebugOptions = 0;
};