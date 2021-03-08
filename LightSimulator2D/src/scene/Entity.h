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
	Vec3 color = VEC3_ZERO;
	float radius = 0.0f;
};

enum class OccluderShape
{
	Rect,
	Mesh
};

struct OcclusionProperties
{
	OccluderShape shape = OccluderShape::Rect;
	unsigned meshLod = 0;
};

class Entity
{
	friend class Renderer;

public:
	Entity(const std::string& texture, const std::string& normal_map = "");
	~Entity();

	void Update(float dt);

	unsigned GetID() { return m_EntityID; }
	
	inline DrawFlags& GetDrawFlags() { return m_DrawFlags; }
	inline EmissionProperties& GetEmissionProperties() { return m_EmissionProperties; }
	inline OcclusionProperties& GetOcclusionProperties() { return m_OcclusionProperties; }
	inline Texture* GetTexture() { return m_Texture; }
	inline Texture* GetNormalMap() { return m_NormalMap; }

	void AddComponent(Component* component);

	inline Mat3 GetTransformation() const
	{
		float angle = -m_Transform.rotation;

		Mat3 scaleTranslation = {
			m_Transform.scale.x,0.0,m_Transform.position.x,
			0.0,m_Transform.scale.y,m_Transform.position.y,
			0.0,0.0,1.0 };

		if (angle == 0.0) return scaleTranslation;

		Mat3 rotation = {
			cos(angle),-sin(angle),0.0,
			sin(angle),cos(angle),0.0,
			0.0,0.0,1.0 };

		return rotation * scaleTranslation;
	}

	Transform m_Transform;
private:
	unsigned m_EntityID;

	std::string m_TexturePath;
	Texture* m_Texture;

	std::string m_NormalMapPath;
	Texture* m_NormalMap = nullptr;

	bool m_ReadyForDraw = false;
	DrawFlags m_DrawFlags = {0};
	EmissionProperties m_EmissionProperties;
	OcclusionProperties m_OcclusionProperties;

	std::vector<Component*> m_Components;
};