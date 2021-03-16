#pragma once

#include "common.h"

#include <vector>

class Texture;
class Renderer;

class Component;

class Entity;

struct Transform
{
	Vec2 position = VEC2_ZERO;
	Vec2 scale = VEC2_ONE;
	float rotation = 0.0f;
};

struct DrawFlags
{
	bool occluder : 1;
	bool emitter : 1;
	bool background : 1;
	bool foreground : 1;
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

struct BackgroundProperties
{
	float textureScale = 1.0;
};

class EntityInstance
{
	// TODO: Make components local to instance
public:
	EntityInstance(Entity* parent) : m_Parent(parent) {}

	inline Vec2 GetPosition() { return m_Transform.position; }

	inline void SetPosition(const Vec2 pos) { m_Transform.position = pos; }

	inline void Move(const Vec2 pos) { m_Transform.position += pos; }
	inline void ApplyRotation(const float rotation) { m_Transform.rotation += rotation; }
	inline void ApplyScale(const float scale) { m_Transform.scale *= scale; }
	inline void ApplyScale(const Vec2 scale) { m_Transform.scale *= scale; }

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

private:
	Entity* m_Parent;
	Transform m_Transform;
};

class Entity
{
	friend class Renderer;

public:
	Entity(const std::string& texture, const std::string& normal_map = "");
	~Entity();

	void Update(float dt);

	unsigned GetID() { return m_EntityID; }
	
	inline std::vector<EntityInstance*>& GetInstances() { return m_Instances; }

	inline DrawFlags& GetDrawFlags() { return m_DrawFlags; }
	inline EmissionProperties& GetEmissionProperties() { return m_EmissionProperties; }
	inline OcclusionProperties& GetOcclusionProperties() { return m_OcclusionProperties; }
	inline BackgroundProperties& GetBackgroundProperties() { return m_BackgroundProperties; }

	inline Texture* GetTexture() { return m_Texture; }
	inline Texture* GetNormalMap() { return m_NormalMap; }

	void AddComponent(Component* component);

	inline EntityInstance* Instance() 
	{ 
		EntityInstance* et = new EntityInstance(this);
		m_Instances.push_back(et);
		return et;
	}

private:
	unsigned m_EntityID;
	bool m_ReadyForDraw = false;

	std::string m_TexturePath;
	Texture* m_Texture;

	std::string m_NormalMapPath;
	Texture* m_NormalMap = nullptr;

	DrawFlags m_DrawFlags = {0};
	EmissionProperties m_EmissionProperties;
	OcclusionProperties m_OcclusionProperties;
	BackgroundProperties m_BackgroundProperties;

	std::vector<Component*> m_Components;

	std::vector<EntityInstance*> m_Instances;
};