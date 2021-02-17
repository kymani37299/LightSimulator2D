#pragma once

#include <vector>
#include <queue>
#include <map>

#include "common.h"
#include "shaders/common.h"

class ShaderStorageBuffer;
class ShaderInput;
class UniformBuffer;
class ComputeShader;
class Scene;
class Entity;

class LightOcclusionRenderer
{
public:
	LightOcclusionRenderer();
	~LightOcclusionRenderer();

	void OnEntityAdded(Entity& e);
	void OnEntityRemoved(Entity& e);

	void RenderOcclusion(Scene* scene);
	unsigned SetupOcclusionMeshInput();

#ifdef GPU_OCCLUSION
	ComputeShader*& GetOcclusionShader() { return m_OcclusionShader; }
	ComputeShader*& GetTriangulateShader() { return m_TriangulationShader; }
#endif
	ComputeShader*& GetOcclusuionMeshGenShader() { return m_OcclusionMeshGenShader; }

private:
	void SetupLineSegments(Scene* scene);
	void SetupRayQuery();
	
	void LightOcclusion(Scene* scene);
	void TriangulateMeshes();

private:

	static constexpr unsigned NUM_ANGLED_RAYS = 360;

	unsigned m_OcclusionLineCount = 0;
	Vec2 m_LightSource;

	ShaderInput* m_OcclusionMesh = nullptr;

	struct RayAngleComparator
	{
		bool operator () (const Vec2& l, const Vec2& r)
		{
			return glm::atan(l.y, l.x) < glm::atan(r.y, r.x);
		}
	};
	using RayQuery = std::priority_queue<Vec2, std::vector<Vec2>, RayAngleComparator>;
	RayQuery m_RayQuery;

	using OcclusionMesh = std::vector<Vec2>;
	std::map<unsigned, OcclusionMesh> m_OcclusionMeshPool;

	ComputeShader* m_OcclusionMeshGenShader = nullptr;
	ShaderStorageBuffer* m_OcclusionMeshOutput;

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