#pragma once

#include "common.h"
#include "GLCore.h"

#include <vector>

class CulledScene;

class DebugRenderer
{
private:
	struct DebugPoint 
	{
		Vec2 pos;
		Vec3 color;
	};

	DebugRenderer() {}
	~DebugRenderer();
	static DebugRenderer* s_Instance;

public:
	static DebugRenderer* Get();
	static void Delete();
	
	void CompileShaders();
	void RenderDebug(CulledScene& scene);

	void DrawPoint(Vec2 position, Vec3 color = {1.0f,0.0f,0.0f});

private:

	static constexpr float POINT_SIZE = 0.01f;

	Shader* m_DebugShader = nullptr;

	std::vector<DebugPoint> m_Points;
};