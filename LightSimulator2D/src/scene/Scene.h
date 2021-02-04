#pragma once

#include "scene/Entity.h"

#include <vector>

class Renderer;

class Scene
{
public:
    inline void AddEntity(const Entity& entity) 
    {
        m_Entites.push_back(entity);
        OnEntityAdded(m_Entites.back());
    }

    inline void RemoveEntity(unsigned id) 
    { 
        int index = FindById(id);
        if (index != -1)
        {
            OnEntityRemoved(m_Entites[index]);
            m_Entites.erase(m_Entites.begin() + index);
        }
    }

    void Init(Renderer* renderer);

    std::vector<Entity>::iterator Begin() { return m_Entites.begin(); }
    std::vector<Entity>::iterator End() { return m_Entites.end(); }

    Entity& operator [] (int index) { return m_Entites[index]; }

    size_t FindById(unsigned id) 
    {
        for (size_t i = 0; i < m_Entites.size(); i++)
        {
            if (m_Entites[i].GetID() == id)
            {
                return i;
            }
        }
        return -1;
    }

private:
    void OnEntityAdded(Entity& entity);
    void OnEntityRemoved(Entity& entity);

private:
    Renderer* m_Renderer;
	std::vector<Entity> m_Entites;
};