#include "Scene.h"

#include "util/Profiler.h"

#include "gfx/Renderer.h"

void RemoveEntityFromVector(Entity* e, std::vector<Entity*>& v)
{
	size_t i = 0;
	for (; i < v.size(); i++)
	{
		if (e->GetID() == v[i]->GetID()) break;
	}

	if (i != v.size()) v.erase(v.begin() + i);
}

Scene::~Scene()
{
	if (m_Renderer)
		m_Renderer->FreeScene();

	for (Entity* e : m_Entites) delete e;
}

void Scene::Init(Renderer* renderer)
{
	m_Renderer = renderer;
	m_Renderer->SetScene(this);
}

void Scene::Update(float dt)
{
	PROFILE_SCOPE("Scene - Update");

	for (Entity* e : m_Entites)
	{
		e->Update(dt);
	}
}

void Scene::OnEntityAdded(Entity* e)
{
	if(m_Renderer)
		m_Renderer->OnEntityAdded(e);

	DrawFlags df = e->GetDrawFlags();
	if (df.occluder) m_Occluders.push_back(e);
	if (df.emitter) m_Emitters.push_back(e);
	if (df.background) m_Background = e;
	if (df.foreground) m_Foreground.push_back(e);
}

void Scene::OnEntityRemoved(Entity* e)
{
	if(m_Renderer)
		m_Renderer->OnEntityRemoved(e);

	DrawFlags df = e->GetDrawFlags();
	if (df.occluder) RemoveEntityFromVector(e, m_Occluders);
	if (df.emitter) RemoveEntityFromVector(e, m_Emitters);
	if (df.background && e == m_Background) m_Background = nullptr;
	if (df.foreground) RemoveEntityFromVector(e, m_Foreground);
}