#pragma once

class Entity;

class Component
{
public:
	virtual void Update(float dt) {};
	virtual void OnAttached(Entity* entity) { m_Parent = entity; };

protected:
	Entity* m_Parent;
};