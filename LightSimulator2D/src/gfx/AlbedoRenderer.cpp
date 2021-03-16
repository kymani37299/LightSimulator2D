#include "AlbedoRenderer.h"

#include "common.h"
#include "gfx/GLCore.h"

#include "scene/Scene.h"
#include "scene/Entity.h"

#include "util/Profiler.h"

extern bool CreateShader(const std::string& name, Shader*& shader);

AlbedoRenderer::AlbedoRenderer()
{
	m_AlbedoFB = new Framebuffer(SCREEN_WIDTH, SCREEN_HEIGHT);
}

AlbedoRenderer::~AlbedoRenderer()
{
	delete m_AlbedoShader;
	delete m_AlbedoFB;
}

void AlbedoRenderer::CompileShaders()
{
	CreateShader("albedo", m_AlbedoShader);
}

void AlbedoRenderer::RenderBackground(Scene* scene)
{
    const Camera& cam = scene->GetCamera();

    PROFILE_SCOPE("Albedo-Background");
    m_AlbedoFB->ClearAndBind();
    m_AlbedoShader->Bind();

    SetupLightSources(scene, false);

    m_AlbedoShader->SetUniform("u_View", MAT3_IDENTITY);
    m_AlbedoShader->SetUniform("u_Transform", MAT3_IDENTITY);

    Entity* bg = scene->GetBackground();
    float texScale = bg->GetBackgroundProperties().textureScale / cam.zoom;
    Vec2 texScale2D = Vec2(texScale * SCREEN_ASPECT_RATIO, texScale);
    m_AlbedoShader->SetUniform("u_UVScale", texScale2D);
    m_AlbedoShader->SetUniform("u_UVOffset", -(cam.position + bg->m_Transform.position) / 2.0f);
    if (bg) RenderEntity(bg);

    m_AlbedoFB->Unbind();
}

void AlbedoRenderer::RenderBase(Scene* scene)
{
    PROFILE_SCOPE("Albedo-Base");
    m_AlbedoFB->Bind();
    m_AlbedoShader->Bind();

    SetupLightSources(scene);

    m_AlbedoShader->SetUniform("u_View", scene->GetCamera().GetTransformation());
    m_AlbedoShader->SetUniform("u_UVScale", VEC2_ONE);
    m_AlbedoShader->SetUniform("u_UVOffset", VEC2_ZERO);

    for (auto it = scene->Begin(); it != scene->End(); it++)
    {
        Entity* e = (*it);
        DrawFlags df = e->GetDrawFlags();
        if (df.background || df.emitter || df.occluder || df.foreground) continue;

        RenderEntity(e);
    }

    m_AlbedoFB->Unbind();
}

void AlbedoRenderer::RenderOccluders(Scene* scene)
{
    PROFILE_SCOPE("Albedo-Occluders");
    m_AlbedoShader->Bind();

    SetupLightSources(scene);

    m_AlbedoShader->SetUniform("u_View", scene->GetCamera().GetTransformation());
    m_AlbedoShader->SetUniform("u_UVScale", VEC2_ONE);
    m_AlbedoShader->SetUniform("u_UVOffset", VEC2_ZERO);

    for (Entity* e : scene->GetOccluders())
    {
        RenderEntity(e);
    }
}

void AlbedoRenderer::RenderForeground(Scene* scene)
{
    PROFILE_SCOPE("Albedo-Foreground");

    m_AlbedoShader->Bind();

    SetupLightSources(scene);

    m_AlbedoShader->SetUniform("u_View", scene->GetCamera().GetTransformation());
    m_AlbedoShader->SetUniform("u_UVScale", VEC2_ONE);
    m_AlbedoShader->SetUniform("u_UVOffset", VEC2_ZERO);

    for (Entity* e : scene->GetForeground())
    {
        RenderEntity(e);
    }
}

void AlbedoRenderer::RenderEntity(Entity* entity)
{
	if (!entity->GetDrawFlags().background) m_AlbedoShader->SetUniform("u_Transform", entity->GetTransformation());

	entity->GetTexture()->Bind(0);

	Texture* normal = entity->GetNormalMap();
	if (normal) normal->Bind(1);
	m_AlbedoShader->SetUniform("u_NormalEnabled", normal != nullptr);

	GLFunctions::DrawPoints(1);
}

void AlbedoRenderer::SetupLightSources(Scene* scene, bool ignoreCam)
{
	Vec2 camPos = ignoreCam ? VEC2_ZERO : scene->GetCamera().position;

	m_AlbedoShader->SetUniform("u_NumLightSources", (int)scene->GetEmitters().size());
	unsigned index = 0;
	for (Entity* e : scene->GetEmitters())
	{
		m_AlbedoShader->SetUniform("u_LightSources[" + std::to_string(index) + "]", e->m_Transform.position + camPos);
		index++;
	}
}