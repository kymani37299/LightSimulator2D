#include "Entity.h"

#include "scene/Component.h"

Entity::Entity(const std::string& texture, const std::string& normal_map)
{
	static unsigned autoInc = 1;
	m_EntityID = autoInc++;

	m_TexturePath = texture;
	m_NormalMapPath = normal_map;
}

Entity::~Entity()
{
	for (Component* c : m_Components)
	{
		delete c;
	}
}

void Entity::AddComponent(Component* component)
{ 
	component->OnAttached(this);
	m_Components.push_back(component); 
}

void Entity::Update(float dt)
{
	for (Component* c : m_Components)
	{
		c->Update(dt);
	}
}