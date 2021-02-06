#include "Entity.h"

Entity::Entity(const std::string& texture)
{
	static unsigned autoInc = 1;
	m_EntityID = autoInc++;

	m_Transform.position = VEC2_ZERO;
	m_Transform.scale = VEC2_ONE;
	m_Transform.rotation = 0.0f;

	m_TexturePath = texture;
}