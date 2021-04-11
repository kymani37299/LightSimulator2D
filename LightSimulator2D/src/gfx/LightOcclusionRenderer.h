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
	Vec2 position = VEC2_ZERO;
	Vec3 color = VEC3_ZERO;
	float radius = 0.0f;
	float strength = 0.0f;
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

	Framebuffer* GetOcclusionMaskFB() { return m_OcclusionMaskFinal; }

private:
	void SetupBuffers(CulledScene& scene);

	using OcclusionMesh = std::vector<Vec2>;
	void PopulateOcclusionMesh(OcclusionMesh& mesh, int meshSize);
	
	void LightOcclusion(CulledScene& scene);
	void TriangulateMeshes();
	void RenderOcclusionMask(CulledScene& scene);
	void BlurMask();

private:

	static constexpr unsigned NUM_ANGLED_RAYS = 30;
	static constexpr unsigned NUM_LIGHT_SAMPLES = 6;

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
	ShaderStorageBuffer* m_TriangledIntersecitonsBuffer;
	ShaderStorageBuffer* m_OcclusionMeshOutput;

	UniformBuffer* m_OcclusionLines;
	UniformBuffer* m_RayQueryBuffer;

	Framebuffer* m_OcclusionMaskFB;
	Framebuffer* m_OcclusionMaskFinal;
};