#include "AlbedoRenderer.h"

#include "common.h"
#include "gfx/GLCore.h"
#include "gfx/SceneCuller.h"

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

void AlbedoRenderer::RenderBackground(CulledScene& scene)
{
    Entity* bg = scene.GetBackground();
    if (bg == nullptr) return;

    const Camera& cam = scene.GetCamera();

    PROFILE_SCOPE("Albedo-Background");
    m_AlbedoFB->ClearAndBind();
    m_AlbedoShader->Bind();

    SetupLightSources(scene, false);
    SetupDefaultParams(scene);

    m_AlbedoShader->SetUniform("u_View", MAT3_IDENTITY);
    m_AlbedoShader->SetUniform("u_Transform", MAT3_IDENTITY);
    m_AlbedoShader->SetUniform("u_DistanceBasedLight", false);

    ASSERT(bg->GetInstances().size() == 1);

    EntityInstance* bgi = bg->GetInstances()[0];
    float texScale = bg->GetBackgroundProperties().textureScale / cam.zoom;
    Vec2 texScale2D = Vec2(texScale * SCREEN_ASPECT_RATIO, texScale);
    m_AlbedoShader->SetUniform("u_UVScale", texScale2D);
    m_AlbedoShader->SetUniform("u_UVOffset", -(cam.position + bgi->GetPosition()) / 2.0f);

    if (bg)
    {
        bg->GetTexture()->Bind(0);
        Texture* normal = bg->GetNormalMap();
        if (normal) normal->Bind(1);
        m_AlbedoShader->SetUniform("u_NormalEnabled", normal != nullptr);
        GLFunctions::DrawPoints(1);
    }

    m_AlbedoFB->Unbind();
}

void AlbedoRenderer::RenderBase(CulledScene& scene)
{
    PROFILE_SCOPE("Albedo-Base");
    m_AlbedoFB->Bind();
    m_AlbedoShader->Bind();

    SetupLightSources(scene);
    SetupDefaultParams(scene);
    m_AlbedoShader->SetUniform("u_DistanceBasedLight", false);

    for (CulledEntity* ce : scene.GetAlbedo())
    {
        Entity* e = ce->GetEntity();
        DrawFlags df = e->GetDrawFlags();
        if (df.background || df.emitter || df.occluder || df.foreground) continue;

        RenderEntity(ce);
    }

    m_AlbedoFB->Unbind();
}

void AlbedoRenderer::RenderOccluders(CulledScene& scene)
{
    PROFILE_SCOPE("Albedo-Occluders");
    m_AlbedoShader->Bind();

    SetupLightSources(scene);
    SetupDefaultParams(scene);

    for (CulledEntity* ce : scene.GetOccluders())
    {
        RenderEntity(ce);
    }
}

void AlbedoRenderer::RenderForeground(CulledScene& scene)
{
    PROFILE_SCOPE("Albedo-Foreground");

    m_AlbedoShader->Bind();

    SetupLightSources(scene);
    SetupDefaultParams(scene);

    for (CulledEntity* ce : scene.GetForeground())
    {
        RenderEntity(ce);
    }
}

void AlbedoRenderer::RenderEntity(CulledEntity* ce)
{
    Entity* e = ce->GetEntity();
    e->GetTexture()->Bind(0);
    Texture* normal = e->GetNormalMap();
    if (normal) normal->Bind(1);
    m_AlbedoShader->SetUniform("u_NormalEnabled", normal != nullptr);

    for (EntityInstance* ei : ce->GetInstances())
    {
        m_AlbedoShader->SetUniform("u_Transform", ei->GetTransformation());
        GLFunctions::DrawPoints(1);
    }
}

void AlbedoRenderer::SetupDefaultParams(CulledScene& scene)
{
    m_AlbedoShader->SetUniform("u_View", scene.GetCamera().GetTransformation());
    m_AlbedoShader->SetUniform("u_UVScale", VEC2_ONE);
    m_AlbedoShader->SetUniform("u_UVOffset", VEC2_ZERO);
    m_AlbedoShader->SetUniform("u_DistanceBasedLight", true);
    m_AlbedoShader->SetUniform("u_AmbientLight", scene.GetAmbientLight());
    m_AlbedoShader->SetUniform("u_Attenuation", scene.GetLightAttenuation());
}

void AlbedoRenderer::SetupLightSources(CulledScene& scene, bool ignoreCam)
{
	Vec2 camPos = ignoreCam ? VEC2_ZERO : scene.GetCamera().position;

	unsigned index = 0;
	for (CulledEntity* ce : scene.GetEmitters())
	{
        for (EntityInstance* ei : ce->GetInstances())
        {
            m_AlbedoShader->SetUniform("u_LightSources[" + std::to_string(index) + "]", ei->GetPosition() + camPos);
            index++;
        }
	}
    m_AlbedoShader->SetUniform("u_NumLightSources", (int)index);
}
