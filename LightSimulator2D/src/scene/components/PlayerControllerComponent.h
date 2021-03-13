#pragma once

#include "common.h"

#include "scene/Component.h"

struct Camera;

class PlayerControllerComponent : public Component
{
public:
	PlayerControllerComponent(Camera& camera) : m_Camera(camera) {}

	void Update(float dt) override;

	Vec2& GetMoveDir() { return m_MoveDir; }
	Vec2& GetCameraDir() { return m_CameraDir; }

private:

	float m_MovementSpeed = 1.0f;
	Vec2 m_MoveDir = VEC2_ZERO;

	Camera& m_Camera;
	float m_CameraSpeed = 1.0f;
	Vec2 m_CameraDir = VEC2_ZERO;
};