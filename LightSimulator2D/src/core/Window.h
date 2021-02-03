#pragma once

#include "common.h"
#include <GLFW/glfw3.h>

class Window
{
public:
	Window(int width, int height, const char* title);
	~Window();

	bool Active();
	void UpdateGraphic();
	void Update(float dt);

	bool KeyPressed(int key);
	Vec2 GetMousePosition();

	inline GLFWwindow* GetWindowHandle() const { return m_Window; }
	inline void* GetProcessAddressHandle() const { return m_ProcAddressHandle; }

	inline bool IsCursorEnabled() const { return m_CursorEnabled; }
	void SetCursorEnabled(bool value);

	inline float GetFps() { return m_Fps; }
	void KeyPressedCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
	void* m_ProcAddressHandle;
	GLFWwindow* m_Window;

	bool m_CursorEnabled;
	float m_Fps = 0.0f;
};

