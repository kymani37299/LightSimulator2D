#pragma once

#include "common.h"

#include <vector>

class Texture;
class Renderer;

class Component;

struct Transform
{
	Vec2 position;
	Vec2 scale;
	float rotation;
};

struct DrawFlags
{
	bool occluder : 1;
	bool emitter : 1;
};

struct EmissionProperties
{
	Vec3 color;
	float radius;
};

class Entity
{
	friend class Renderer;

public:
	Entity(const std::string& texture);
	~Entity();

	void Update(float dt);

	unsigned GetID() { return m_EntityID; }
	
	inline DrawFlags& GetDrawFlags() { return m_DrawFlags; }
	inline EmissionProperties& GetEmissionProperties() { return m_EmissionProperties; }
	inline Texture* GetTexture() { return m_Texture; }

	void AddComponent(Component* component);

	// TODO: Apply rotation
	inline Mat3 GetTransformation() const
	{
		return Mat3({
			m_Transform.scale.x,0.0,m_Transform.position.x,
			0.0,m_Transform.scale.y,m_Transform.position.y,
			0.0,0.0,1.0 });
	}

	Transform m_Transform;
private:
	unsigned m_EntityID;

	std::string m_TexturePath;
	Texture* m_Texture;

	bool m_ReadyForDraw = false;
	DrawFlags m_DrawFlags = {0};
	EmissionProperties m_EmissionProperties{ VEC3_ZERO,0 };

	std::vector<Component*> m_Components;
};