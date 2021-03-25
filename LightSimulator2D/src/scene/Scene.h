#pragma once

#include "scene/Entity.h"

#include <vector>

class Renderer;

struct Camera
{
    Vec2 position = VEC2_ZERO;

    // [TODO] - Not used for now
    float rotation = 0.0;
    float zoom = 1.0;

    inline Mat3 GetTransformation() const
    {
        float angle = -rotation;

        Mat3 scaleTranslation = {
            zoom,0.0,position.x,
            0.0,zoom,position.y,
            0.0,0.0,1.0 };

        if (angle == 0.0) return scaleTranslation;

        Mat3 rotation = {
            cos(angle),-sin(angle),0.0,
            sin(angle),cos(angle),0.0,
            0.0,0.0,1.0 };

        return glm::inverse(rotation * scaleTranslation);
    }

    inline Vec2 GetViewSpacePosition(Vec2 position) const
    {
        Vec3 tmp = Vec3(position.x, position.y, 1.0f);
        tmp = tmp * GetTransformation();
        return Vec2(tmp.x,tmp.y);
    }
};

class Scene
{
public:
    ~Scene();

    inline void AddEntity(Entity* entity) 
    {
        m_Entites.push_back(entity);
        OnEntityAdded(entity);
    }

    inline void RemoveEntity(unsigned id) 
    { 
        int index = FindById(id);
        if (index != -1)
        {
            Entity* e = m_Entites[index];
            OnEntityRemoved(e);
            m_Entites.erase(m_Entites.begin() + index);
            delete e;
        }
    }

    void Init(Renderer* renderer);
    void Update(float dt);

    std::vector<Entity*>::iterator Begin() { return m_Entites.begin(); }
    std::vector<Entity*>::iterator End() { return m_Entites.end(); }
    size_t Size() { return m_Entites.size(); }

    Entity* operator [] (int index) { return m_Entites[index]; }

    inline std::vector<Entity*>& GetOccluders() { return m_Occluders; }
    inline std::vector<Entity*>& GetEmitters() { return m_Emitters; }
    inline std::vector<Entity*>& GetForeground() { return m_Foreground; }
    inline Entity* GetBackground() { return m_Background; }
    inline Camera& GetCamera() { return m_Camera; }
    inline Vec3& GetAmbientLight() { return m_AmbientLight; }
    inline Vec3& GetLightAttenuation() { return m_LightAttenuation; }

    size_t FindById(unsigned id) 
    {
        for (size_t i = 0; i < m_Entites.size(); i++)
        {
            if (m_Entites[i]->GetID() == id)
            {
                return i;
            }
        }
        return -1;
    }

private:
    void OnEntityAdded(Entity* entity);
    void OnEntityRemoved(Entity* entity);

private:
    Renderer* m_Renderer;

    // For now scene is vector of pointers, too much trouble otherwise
	std::vector<Entity*> m_Entites;

    std::vector<Entity*> m_Occluders;
    std::vector<Entity*> m_Emitters;
    std::vector<Entity*> m_Foreground;
    Entity* m_Background;
    Camera m_Camera;
    Vec3 m_AmbientLight = VEC3_ZERO;
    Vec3 m_LightAttenuation = Vec3(0.4f, 1.0f, 5.0f);
};