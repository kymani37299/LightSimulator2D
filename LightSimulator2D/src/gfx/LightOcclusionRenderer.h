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
	
	void CompileShaders();

	void OnOccluderAdded(Entity& e);
	void OnOccluderRemoved(Entity& e);

	void RenderOcclusion();
	void BindOcclusionMask(unsigned slot);

private:
	void SetupLineSegments();
	void SetupRayQuery();
	unsigned SetupOcclusionMeshInput();

	using OcclusionMesh = std::vector<Vec2>;
	void PopulateOcclusionMesh(OcclusionMesh& mesh);
	
	void LightOcclusion();
	void TriangulateMeshes();
	void RenderOcclusionMask();

private:

	static constexpr unsigned NUM_ANGLED_RAYS = 360;

	Vec2 m_LightSource;

	std::map<Entity*, OcclusionMesh> m_OcclusionMeshPool;

	unsigned m_OcclusionLineCount = 0;
	ShaderInput* m_OcclusionMesh = nullptr;

	ShaderStorageBuffer* m_OcclusionMeshOutput;

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

#ifdef GPU_OCCLUSION
	unsigned m_RayCount = 0;

	ShaderStorageBuffer* m_IntersectionBuffer;
	ShaderStorageBuffer* m_TriangledIntersecitonsBuffer;
	UniformBuffer* m_OcclusionLines;
	UniformBuffer* m_RayQueryBuffer;

	ComputeShader* m_OcclusionShader = nullptr;
	ComputeShader* m_TriangulationShader = nullptr;
#else
	std::vector<Vec2> m_Intersections;
	std::vector<Vec2> m_TriangledIntersections;
	std::vector<Vec4> m_Segments;
#endif
};