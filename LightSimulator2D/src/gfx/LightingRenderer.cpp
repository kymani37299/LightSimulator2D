#include "LightingRenderer.h"

#include "gfx/GLCore.h"

#include "scene/Scene.h"
#include "util/Profiler.h"

extern bool CreateShader(const std::string& name, Shader*& shader);

LightingRenderer::LightingRenderer(Framebuffer* albedoFB, Framebuffer* occlusionFB):
	m_AlbedoFB(albedoFB),
	m_OcclusionFB(occlusionFB)
{
}

LightingRenderer::~LightingRenderer()
{
    delete m_OccluderShader;
	delete m_LightingShader;
}

void LightingRenderer::CompileShaders()
{
    static std::string shader_path = "lighting/";

    CreateShader(shader_path + "lighting", m_LightingShader);
    CreateShader(shader_path + "emitter", m_EmitterShader);
    CreateShader(shader_path + "occluder", m_OccluderShader);
}

void LightingRenderer::RenderLights(Scene* scene)
{
    RenderLighting();
    RenderOccluders(scene);
    RenderEmitters(scene);
}

void LightingRenderer::RenderLighting()
{
    PROFILE_SCOPE("Lighting");

    GLFunctions::MemoryBarrier(BarrierType::Framebuffer);

    m_LightingShader->Bind();
    m_AlbedoFB->BindTexture(0, 0);
    m_OcclusionFB->BindTexture(0, 1);
    GLFunctions::DrawFC();
}

void LightingRenderer::RenderOccluders(Scene* scene)
{
    PROFILE_SCOPE("Draw occluders");
    m_OccluderShader->Bind();

    // TODO: No emitters and multiple emitters
    Vec2 lightSource = scene->GetEmitters()[0]->m_Transform.position;
    m_OccluderShader->SetUniform("u_LightSource", lightSource);

    GLConstants::QuadInput->Bind();
    for (Entity* e : scene->GetOccluders())
    {
        m_OccluderShader->SetUniform("u_Transform", e->GetTransformation());
        e->GetTexture()->Bind(0);

        Texture* normalMap = e->GetNormalMap();
        if (normalMap) normalMap->Bind(1);
        m_OccluderShader->SetUniform("u_NormalEnabled", normalMap != nullptr);

        GLFunctions::Draw(6);
    }
}

void LightingRenderer::RenderEmitters(Scene* scene)
{
    PROFILE_SCOPE("Draw emitters");

    m_EmitterShader->Bind();
    for (Entity* e : scene->GetEmitters())
    {
        m_EmitterShader->SetUniform("u_Transform", e->GetTransformation());
        e->GetTexture()->Bind(0);
        GLFunctions::DrawPoints(1);
    }
}