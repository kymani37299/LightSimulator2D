#include "Entity.h"

#include "scene/Component.h"

Entity::Entity(const std::string& texture)
{
	static unsigned autoInc = 1;
	m_EntityID = autoInc++;

	m_Transform.position = VEC2_ZERO;
	m_Transform.scale = VEC2_ONE;
	m_Transform.rotation = 0.0f;

	m_TexturePath = texture;
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