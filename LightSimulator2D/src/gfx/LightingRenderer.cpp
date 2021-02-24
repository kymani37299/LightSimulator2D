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
    delete m_OpaqueShader;
	delete m_LightingShader;
}

void LightingRenderer::CompileShaders()
{
	CreateShader("lighting", m_LightingShader);
	CreateShader("main", m_OpaqueShader);
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
    m_OpaqueShader->Bind();
    GLConstants::QuadInput->Bind();
    for (auto it = scene->Begin(); it != scene->End(); it++)
    {
        Entity& e = (*it);
        if (!e.GetDrawFlags().occluder) continue;
        m_OpaqueShader->SetUniform("u_Transform", e.GetTransformation());
        e.m_Texture->Bind(0);
        GLFunctions::Draw(6);
    }
}