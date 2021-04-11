#include "SceneCuller.h"

#include "util/Profiler.h"

#include "ui/elements/SceneUI.h"

inline static bool ShouldBeCulled(const Mat3& transformation, const Mat3& cameraTransform, float occlusionFactor)
{
    constexpr Vec4 screenQuad = { -1.0,-1.0,1.0,1.0 };
    const Vec4 scaledQuad = screenQuad * occlusionFactor;
    const Vec3 zoneA = Vec3(scaledQuad.x, scaledQuad.y,1.0) * cameraTransform;
    const Vec3 zoneB = Vec3(scaledQuad.z, scaledQuad.w,1.0) * cameraTransform;
    const Vec3 instanceA = Vec3(screenQuad.x, screenQuad.y, 1.0) * transformation;
    const Vec3 instanceB = Vec3(screenQuad.z, screenQuad.w, 1.0) * transformation;

    const float minX = MIN(instanceA.x, instanceB.x);
    const float minY = MIN(instanceA.y, instanceB.y);
    const float maxX = MAX(instanceA.x, instanceB.x);
    const float maxY = MAX(instanceA.y, instanceB.y);

    return minX >= zoneB.x || zoneA.x >= maxX 
        || minY >= zoneB.y || zoneA.y >= maxY;
}

inline static float GetSpecificOcclusionFactor(DrawFlags df)
{
    if (df.occluder)            return 1.5f;
    else if (df.emitter)        return 1.5f;
    else if (df.foreground)     return 1.0f;
    return -1.0f;
}

void SceneCuller::CullEntity(Entity* e)
{
    Mat3 camTrans = glm::inverse(m_CulledScene.m_Scene->GetCamera().GetTransformation());

    CulledEntity* ce = new CulledEntity(e);
    CulledEntity* ceSpecific = new CulledEntity(e);
    float occlusionFactor = GetSpecificOcclusionFactor(e->GetDrawFlags());

    for (EntityInstance* ei : e->GetInstances())
    {
        m_CulledScene.m_TotalInstances++;

        if (!ShouldBeCulled(ei->GetTransformation(), camTrans, 1.0f))
        {
            ce->m_Instances.push_back(ei);
        }

        if (occlusionFactor > 0.0f && !ShouldBeCulled(ei->GetTransformation(), camTrans, occlusionFactor))
        {
            ceSpecific->m_Instances.push_back(ei);
        }
    }

    if (!ce->m_Instances.empty()) m_CulledScene.m_Albedo.push_back(ce);
    if (!ceSpecific->m_Instances.empty())
    {
        DrawFlags df = e->GetDrawFlags();
        if (df.occluder)        m_CulledScene.m_Occluders.push_back(ceSpecific);
        else if (df.emitter)    m_CulledScene.m_Emitters.push_back(ceSpecific);
        else if (df.foreground) m_CulledScene.m_Foreground.push_back(ceSpecific);
    }

    // Scene stats

    DrawFlags df = e->GetDrawFlags();
    if (df.occluder) m_CulledScene.m_TotalOccluders += e->GetInstances().size();
    if (df.emitter) m_CulledScene.m_TotalEmitters += e->GetInstances().size();
    m_CulledScene.m_TotalInstances += e->GetInstances().size();
    
    if (!ce->m_Instances.empty()) m_CulledScene.m_DrawnInstances += ce->GetInstances().size();
    if (!ceSpecific->m_Instances.empty())
    {
        DrawFlags df = e->GetDrawFlags();
        if (df.occluder)        m_CulledScene.m_DrawnOccluders += ceSpecific->GetInstances().size();
        else if (df.emitter)    m_CulledScene.m_DrawnEmitters += ceSpecific->GetInstances().size();
    }

    //
}

void UpdateSceneUI(Scene* scene, CulledScene* culledScene)
{
    SceneUI* s = SceneUI::Get();
    if (!s) return;
    s->SetTotalEntities(scene->Size());
    s->SetTotalInstances(culledScene->GetTotalInstances());
    s->SetDrawnInstances(culledScene->GetDrawnInstances());
    s->SetTotalEmitters(culledScene->GetTotalEmitters());
    s->SetDrawnEmitters(culledScene->GetDrawnEmitters());
    s->SetTotalOccluders(culledScene->GetTotalOccluders());
    s->SetDrawnOccluders(culledScene->GetDrawnOccluders());
}

CulledScene& SceneCuller::GetCulledScene(Scene* scene)
{
    PROFILE_SCOPE("Culling");

    m_CulledScene.Clear();
    m_CulledScene.m_Scene = scene;

    for (auto it = scene->Begin(); it != scene->End(); it++)
    {
        CullEntity((*it));
    }

    UpdateSceneUI(scene, &m_CulledScene);

    return m_CulledScene;
}
