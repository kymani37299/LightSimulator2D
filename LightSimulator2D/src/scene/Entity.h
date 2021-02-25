#pragma once

#include "common.h"

class Texture;
class Renderer;

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

class Entity
{
	friend class Renderer;

public:
	Entity(const std::string& texture);
	unsigned GetID() { return m_EntityID; }
	
	inline DrawFlags& GetDrawFlags() { return m_DrawFlags; }
	inline Texture* GetTexture() { return m_Texture; }

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
};