#pragma once

#include <vector>

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

	ComputeShader*& GetOcclusionShader() { return m_OcclusionShader; }
	ComputeShader*& GetTriangulateShader() { return m_TriangulationShader; }

	ShaderInput* GetOcclusionMesh() { return m_TriangledIntersecitonsShaderInput; }
	unsigned GetOccusionMeshVertexNumber() { return NUM_TRIANGLED_INTERSECTION_VERTICES; }

	void SetUseGPU(bool value);

private:
	void LightOcclusionGPU(Scene* scene);
	void TriangulateMeshesGPU();

	void LightOcclusionCPU(Scene* scene);
	void TriangulateMeshesCPU();

	void InitGPUResources();
	void DeleteGPUResources();

private:
	bool m_UseGPU = false;

	static constexpr unsigned NUM_INTERSECTIONS = 30;
	ShaderStorageBuffer* m_IntersectionBuffer;

	UniformBuffer* m_OcclusionLines;
	unsigned m_OcclusionLineCount = 0;

	static constexpr unsigned NUM_TRIANGLED_INTERSECTION_VERTICES = (NUM_INTERSECTIONS - 1) * 3;
	ShaderStorageBuffer* m_TriangledIntersecitonsBuffer;
	ShaderInput* m_TriangledIntersecitonsShaderInput = nullptr;

	ComputeShader* m_OcclusionShader = nullptr;
	ComputeShader* m_TriangulationShader = nullptr;

	Vec2 m_LightPosition = Vec2(-0.5, -0.5);
	std::vector<Vec2> m_Intersections{ NUM_INTERSECTIONS };
};