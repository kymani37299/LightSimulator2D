#include "LightingRenderer.h"

#include "gfx/GLCore.h"
#include "gfx/SceneCuller.h"

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
    CreateShader("albedo", m_OccluderShader);
}

void LightingRenderer::RenderLighting(CulledScene& scene)
{
    PROFILE_SCOPE("Lighting");

    GLFunctions::MemoryBarrier(BarrierType::Framebuffer);

    m_LightingShader->Bind();
    m_AlbedoFB->BindTexture(0, 0);
    m_OcclusionFB->BindTexture(0, 1);
    m_LightingShader->SetUniform("u_AmbientLight", scene.GetAmbientLight());
    GLFunctions::DrawFC();
}

void LightingRenderer::RenderEmitters(CulledScene& scene)
{
    PROFILE_SCOPE("Draw emitters");

    GLFunctions::AlphaBlending(true); // Adding blurred textures
    m_EmitterShader->Bind();
    m_EmitterShader->SetUniform("u_View", scene.GetCamera().GetTransformation());
    for (CulledEntity* ce : scene.GetEmitters())
    {
        Entity* e = ce->GetEntity();
        m_EmitterShader->SetUniform("u_EmitterColor", e->GetEmissionProperties().color);
        e->GetTexture()->Bind(0);
        for (EntityInstance* ei : ce->GetInstances())
        {
            m_EmitterShader->SetUniform("u_Transform", ei->GetTransformation());
            GLFunctions::DrawPoints(1);
        }
    }
    GLFunctions::AlphaBlending(false);
}