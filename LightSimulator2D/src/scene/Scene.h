#pragma once

#include "scene/Entity.h"

#include <vector>

class Scene
{
public:
    inline void AddEntity(const Entity& entity) { m_Entites.push_back(entity); m_Dirty = true; }
    inline void ClearDirty() { m_Dirty = false; }
    inline bool IsDirty() { return m_Dirty; }

    std::vector<Entity>::iterator Begin() { return m_Entites.begin(); }
    std::vector<Entity>::iterator End() { return m_Entites.end(); }

private:
	std::vector<Entity> m_Entites;
	bool m_Dirty = false;
};