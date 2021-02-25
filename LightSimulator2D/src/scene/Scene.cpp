#include "Scene.h"

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
}

void Scene::OnEntityRemoved(Entity* e)
{
	if(m_Renderer)
		m_Renderer->OnEntityRemoved(e);

	DrawFlags df = e->GetDrawFlags();
	if (df.occluder) RemoveEntityFromVector(e, m_Occluders);
	if (df.emitter) RemoveEntityFromVector(e, m_Emitters);
}