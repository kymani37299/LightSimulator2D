#include "DebugRenderer.h"

#include "gfx/SceneCuller.h"

extern bool CreateShader(const std::string& name, Shader*& shader);

DebugRenderer* DebugRenderer::s_Instance = nullptr;

DebugRenderer::~DebugRenderer()
{
	SAFE_DELETE(m_PointShader);
	SAFE_DELETE(m_LineShader);
}

DebugRenderer* DebugRenderer::Get()
{
	if (!s_Instance)
	{
		s_Instance = new DebugRenderer();
	}
	return s_Instance;
}

void DebugRenderer::Delete()
{
	SAFE_DELETE(s_Instance);
}

void DebugRenderer::CompileShaders()
{
	CreateShader("debug/point", m_PointShader);
	CreateShader("debug/line", m_LineShader);
}

void DebugRenderer::RenderDebug(CulledScene& scene)
{
	static Entity e{ "" };
	static EntityInstance* ei = e.Instance();

	// Lines
	m_LineShader->Bind();
	m_LineShader->SetUniform("u_View", scene.GetCamera().GetTransformation());
	GLFunctions::AlphaBlending(true);

	for (DebugLine dl : m_Lines)
	{
		m_LineShader->SetUniform("u_Begin", dl.begin);
		m_LineShader->SetUniform("u_End", dl.end);
		m_LineShader->SetUniform("u_Color", dl.color);
		GLFunctions::DrawFC();
	}
	GLFunctions::AlphaBlending(false);
	m_Lines.clear();

	// Points
	m_PointShader->Bind();
	m_PointShader->SetUniform("u_View", scene.GetCamera().GetTransformation());

	ei->SetScale(Vec2(POINT_SIZE, POINT_SIZE * SCREEN_ASPECT_RATIO));
	for (DebugPoint dp : m_Points)
	{
		ei->SetPosition(dp.pos);
		m_PointShader->SetUniform("u_Transform", ei->GetTransformation());
		m_PointShader->SetUniform("u_Color", dp.color);
		GLFunctions::DrawPoints(1);
	}
	m_Points.clear();
}

void DebugRenderer::DrawPoint(Vec2 position, Vec3 color)
{
	m_Points.push_back({ position,color });
}

void DebugRenderer::DrawLine(Vec2 begin, Vec2 end, Vec3 color)
{
	m_Lines.push_back({ begin,end,color });
}
