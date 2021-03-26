#include "SceneCuller.h"

#include "util/Profiler.h"

#include "gfx/DebugRenderer.h"

inline static bool InRect(Vec2 x, Vec2 a, Vec2 b)
{
    return x.x > a.x && x.x < b.x && x.y > a.y&& x.y < b.y;
}

inline static bool ShouldBeCulled(const Mat3& transformation, const Mat3& cameraTransform, float occlusionFactor)
{
    static const Vec4 screenQuad = { -1.0,-1.0,1.0,1.0 };
    const Vec3 zoneA = (Vec3(screenQuad.x, screenQuad.y,1.0/occlusionFactor) * occlusionFactor) * cameraTransform;
    const Vec3 zoneB = (Vec3(screenQuad.z, screenQuad.w,1.0/occlusionFactor) * occlusionFactor) * cameraTransform;
    const Vec3 instanceA = Vec3(screenQuad.x, screenQuad.y, 1.0) * transformation;
    const Vec3 instanceB = Vec3(screenQuad.z, screenQuad.w, 1.0) * transformation;

    return instanceA.x >= zoneB.x || zoneA.x >= instanceB.x 
        || instanceA.y >= zoneB.y || zoneA.y >= instanceB.y;
}

inline static float GetSpecificOcclusionFactor(DrawFlags df)
{
    if (df.occluder)            return 1.5f;
    else if (df.emitter)        return 2.0f;
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
        if (df.occluder)            m_CulledScene.m_Occluders.push_back(ce);
        else if (df.emitter)        m_CulledScene.m_Emitters.push_back(ce);
        else if (df.foreground)     m_CulledScene.m_Foreground.push_back(ce);
    }
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

    return m_CulledScene;
}
