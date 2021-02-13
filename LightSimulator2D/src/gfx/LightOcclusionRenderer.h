#pragma once

#include <vector>
#include <queue>

#include "common.h"

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

#ifdef GPU_OCCLUSION
	ComputeShader*& GetOcclusionShader() { return m_OcclusionShader; }
	ComputeShader*& GetTriangulateShader() { return m_TriangulationShader; }
#endif

	ShaderInput* GetOcclusionMesh() { return m_TriangledIntersecitonsShaderInput; }

private:
	void SetupLineSegments(Scene* scene);
	void SetupRayQuery();
	
	void LightOcclusion(Scene* scene);
	void TriangulateMeshes();
	
private:

	static constexpr unsigned NUM_INTERSECTIONS = 360;
	unsigned m_OcclusionLineCount = 0;
	static constexpr unsigned NUM_TRIANGLED_INTERSECTION_VERTICES = NUM_INTERSECTIONS * 3;
	Vec2 m_LightSource;

	ShaderInput* m_TriangledIntersecitonsShaderInput = nullptr;

#ifdef GPU_OCCLUSION
	ShaderStorageBuffer* m_IntersectionBuffer;
	ShaderStorageBuffer* m_TriangledIntersecitonsBuffer;
	UniformBuffer* m_OcclusionLines;

	ComputeShader* m_OcclusionShader = nullptr;
	ComputeShader* m_TriangulationShader = nullptr;
#else
	std::vector<Vec2> m_Intersections{ NUM_INTERSECTIONS };
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