#include "LightingRenderer.h"

#include "gfx/GLCore.h"

#include "scene/Scene.h"
#include "util/Profiler.h"

extern bool CreateShader(const std::string& name, Shader*& shader);
extern bool CreateCShader(const std::string& name, ComputeShader*& shader);

LightingRenderer::LightingRenderer(Framebuffer* albedoFB, Framebuffer* occlusionFB):
	m_AlbedoFB(albedoFB),
	m_OcclusionFB(occlusionFB)
{

}

LightingRenderer::~LightingRenderer()
{
    delete m_AlbedoShader;
	delete m_LightingShader;
}

void LightingRenderer::CompileShaders()
{
    static std::string shader_path = "lighting/";

    CreateShader(shader_path + "lighting", m_LightingShader);
    CreateShader("albedo", m_AlbedoShader);
}

void LightingRenderer::RenderLights(Scene* scene)
{
    RenderLighting();
    RenderOccluders(scene);
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
    m_AlbedoShader->Bind();
    GLConstants::QuadInput->Bind();
    for (Entity* e : scene->GetOccluders())
    {
        m_AlbedoShader->SetUniform("u_Transform", e->GetTransformation());
        e->GetTexture()->Bind(0);
        GLFunctions::Draw(6);
    }
}