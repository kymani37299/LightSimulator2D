#pragma once

#include "scene/Entity.h"

#include <vector>

class Renderer;

struct Camera
{
    Vec2 position = VEC2_ZERO;
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
    inline Entity* GetBackground() { return m_Background; }
    inline Camera& GetCamera() { return m_Camera; }

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
    Entity* m_Background;
    Camera m_Camera;
};