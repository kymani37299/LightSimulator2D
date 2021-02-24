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

void LightingRenderer::OnEntityAdded(Entity& entity)
{
    if (entity.m_DrawFlags.occluder)
    {
        m_Occluders.push_back(entity);
    }

    if (entity.m_DrawFlags.emitter)
    {
        m_Emitters.push_back(entity);
    }
}

void RemoveFromVector(Entity& e, std::vector<Entity>& v)
{
    size_t i = 0;
    for (; i < v.size(); i++)
    {
        if (e.GetID() == v[i].GetID()) break;
    }

    if (i != v.size()) v.erase(v.begin() + i);
}

void LightingRenderer::OnEntityRemoved(Entity& entity)
{
    if (entity.m_DrawFlags.occluder)
    {
        RemoveFromVector(entity,m_Occluders);
    }

    if (entity.m_DrawFlags.emitter)
    {
        RemoveFromVector(entity, m_Emitters);
    }
}

void LightingRenderer::RenderLights()
{
    RenderLighting();
    RenderOccluders();
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

void LightingRenderer::RenderOccluders()
{
    PROFILE_SCOPE("Draw occluders");
    m_AlbedoShader->Bind();
    GLConstants::QuadInput->Bind();
    for (Entity& e : m_Occluders)
    {
        m_AlbedoShader->SetUniform("u_Transform", e.GetTransformation());
        e.m_Texture->Bind(0);
        GLFunctions::Draw(6);
    }
}