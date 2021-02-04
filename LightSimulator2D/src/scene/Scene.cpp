#include "Scene.h"

#include "gfx/Renderer.h"

Scene::~Scene()
{
	if (m_Renderer)
		m_Renderer->FreeScene();
}

void Scene::Init(Renderer* renderer)
{
	m_Renderer = renderer;
	m_Renderer->SetScene(this);
}

void Scene::OnEntityAdded(Entity& e)
{
	if(m_Renderer)
		m_Renderer->OnEntityAdded(e);
}

void Scene::OnEntityRemoved(Entity& e)
{
	if(m_Renderer)
		m_Renderer->OnEntityRemoved(e);
}