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

	struct DebugLine
	{
		Vec2 begin;
		Vec2 end;
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
	void DrawLine(Vec2 begin, Vec2 end, Vec3 color = { 1.0f,0.0f,0.0f });

	void DrawFramebuffer(Framebuffer* framebuffer, unsigned slot = 0) { m_Framebuffer = framebuffer; m_FramebufferSlot = slot; }

private:

	static constexpr float POINT_SIZE = 0.005f;

	Shader* m_PointShader = nullptr;
	Shader* m_LineShader = nullptr;
	Shader* m_ScreenShader = nullptr;

	std::vector<DebugPoint> m_Points;
	std::vector<DebugLine> m_Lines;
	Framebuffer* m_Framebuffer = nullptr;
	unsigned m_FramebufferSlot = 0;
};