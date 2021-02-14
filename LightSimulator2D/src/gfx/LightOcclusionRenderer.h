#pragma once

#include <vector>
#include <queue>

#include "common.h"
#include "shaders/common.h"

class ShaderStorageBuffer;
class ShaderInput;
class UniformBuffer;
class ComputeShader;
class Scene;

class LightOcclusionRenderer
{
public:
	LightOcclusionRenderer();
	~LightOcclusionRenderer();

	void RenderOcclusion(Scene* scene);
	unsigned SetupOcclusionMeshInput();

#ifdef GPU_OCCLUSION
	ComputeShader*& GetOcclusionShader() { return m_OcclusionShader; }
	ComputeShader*& GetTriangulateShader() { return m_TriangulationShader; }
#endif

private:
	void SetupLineSegments(Scene* scene);
	void SetupRayQuery();
	
	void LightOcclusion(Scene* scene);
	void TriangulateMeshes();

private:
	unsigned m_OcclusionLineCount = 0;
	Vec2 m_LightSource;

	ShaderInput* m_OcclusionMesh = nullptr;

#ifdef GPU_OCCLUSION
	unsigned m_RayCount = 0;

	ShaderStorageBuffer* m_IntersectionBuffer;
	ShaderStorageBuffer* m_TriangledIntersecitonsBuffer;
	UniformBuffer* m_OcclusionLines;
	UniformBuffer* m_RayQueryBuffer;

	ComputeShader* m_OcclusionShader = nullptr;
	ComputeShader* m_TriangulationShader = nullptr;
#else
	std::vector<Vec2> m_Intersections{ NUM_INTERSECTIONS };
	std::vector<Vec2> m_TriangledIntersections;
	std::vector<Vec4> m_Segments;
	
	// Vec2 of Vec4 for ray query ? What about multiple lights ?
	struct RayAngleComparator
	{
		bool operator () (const Vec2& l, const Vec2& r)
		{
			return glm::atan(l.y, l.x) < glm::atan(r.y, r.x);
		}
	};
	using RayQuery = std::priority_queue<Vec2, std::vector<Vec2>, RayAngleComparator>;
	RayQuery m_RayQuery;
#endif
};