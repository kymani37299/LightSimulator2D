#include "DebugRenderer.h"

#include "gfx/SceneCuller.h"

extern bool CreateShader(const std::string& name, Shader*& shader);

DebugRenderer* DebugRenderer::s_Instance = nullptr;

DebugRenderer::~DebugRenderer()
{
	delete m_DebugShader;
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
	CreateShader("debug/debug", m_DebugShader);
}

void DebugRenderer::RenderDebug(CulledScene& scene)
{
	static Entity e{""};
	static EntityInstance* ei = e.Instance();

	m_DebugShader->Bind();
	m_DebugShader->SetUniform("u_View", scene.GetCamera().GetTransformation());

	ei->SetScale(Vec2(POINT_SIZE, POINT_SIZE * SCREEN_ASPECT_RATIO));
	for (DebugPoint dp : m_Points)
	{
		ei->SetPosition(dp.pos);
		m_DebugShader->SetUniform("u_Transform", ei->GetTransformation());
		m_DebugShader->SetUniform("u_Color", dp.color);
		GLFunctions::DrawPoints(1);
	}
	m_Points.clear();
}

void DebugRenderer::DrawPoint(Vec2 position, Vec3 color)
{
	m_Points.push_back({ position,color });
}
