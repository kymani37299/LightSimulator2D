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

struct DrawParams
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

	Transform m_Transform;
private:
	unsigned m_EntityID;

	std::string m_TexturePath;
	Texture* m_Texture;

	bool m_ReadyForDraw = false;
};