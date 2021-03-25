#pragma once

#include "scene/Scene.h"

class Entity;
class EntityInstance;

class Scene;

class CulledEntity
{
    friend class SceneCuller;

public:
    CulledEntity(Entity* e) : m_Entity(e) {}

    std::vector<EntityInstance*> GetInstances() { return m_Instances; }
    Entity* GetEntity() { return m_Entity; }

private:
    Entity* m_Entity;
    std::vector<EntityInstance*> m_Instances;
};

class CulledScene
{
    friend class SceneCuller;

public:

    ~CulledScene()
    {
        Clear();
    }

    inline void Clear()
    {
        m_Occluders.clear();
        m_Emitters.clear();
        m_Foreground.clear();
        for (CulledEntity* ce : m_Albedo)
        {
            delete ce;
        }
        m_Albedo.clear();

        m_Scene = nullptr;
    }

    inline std::vector<CulledEntity*>& GetOccluders() { return m_Occluders; }
    inline std::vector<CulledEntity*>& GetEmitters() { return m_Emitters; }
    inline std::vector<CulledEntity*>& GetForeground() { return m_Foreground; }
    inline std::vector<CulledEntity*>& GetAlbedo() { return m_Albedo; }
    inline Entity* GetBackground() { return m_Scene->GetBackground(); }
    inline Camera& GetCamera() { return m_Scene->GetCamera(); }
    inline Vec3& GetAmbientLight() { return m_Scene->GetAmbientLight(); }
    inline Vec3& GetLightAttenuation() { return m_Scene->GetLightAttenuation(); }

private:
    std::vector<CulledEntity*> m_Occluders;
    std::vector<CulledEntity*> m_Emitters;
    std::vector<CulledEntity*> m_Foreground;
    std::vector<CulledEntity*> m_Albedo;

    Scene* m_Scene;
};

class SceneCuller
{
public:

	CulledScene& GetCulledScene(Scene* scene);

private:
	void CullEntity(Entity* e);

private:
	CulledScene m_CulledScene;
};