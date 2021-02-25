#include "Window.h"

#include "common.h"
#include "input/Input.h"

static float timeAccumulator = 0.0001f;

void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

Window::Window(int width, int height, const char* title)
{
	// Init glfw
	ASSERT_RUN(glfwInit());

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create window
	m_Window = glfwCreateWindow(width, height, title, NULL, NULL);
	ASSERT(m_Window);

	glfwMakeContextCurrent(m_Window);
	//SetCursorEnabled(false);
	glfwSetKeyCallback(m_Window, &glfwKeyCallback);
	
	m_ProcAddressHandle = glfwGetProcAddress;
}

Window::~Window()
{
	glfwTerminate();
}

bool Window::Active()
{
	return !glfwWindowShouldClose(m_Window);
}

void Window::UpdateGraphic()
{
	m_Fps = 1.0f / timeAccumulator * 1000.0f;
	timeAccumulator = 0.0001f;
	glfwSwapBuffers(m_Window);
}

void Window::Update(float dt)
{
	timeAccumulator += dt;
	glfwPollEvents();
}

bool Window::KeyPressed(int key)
{
	return glfwGetKey(m_Window, key);
}

Vec2 Window::GetMousePosition()
{
	double xpos, ypos;
	glfwGetCursorPos(m_Window, &xpos, &ypos);
	return {xpos,ypos};
}

void Window::SetCursorEnabled(bool value)
{
	m_CursorEnabled = value;
	glfwSetInputMode(m_Window, GLFW_CURSOR, value ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

void Window::KeyPressedCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (m_Input)
	{
		KeyState state = KeyState::Invalid;
		switch (action)
		{
		case GLFW_PRESS:
			state = KeyState::Pressed;
			break;
		case GLFW_REPEAT:
			state = KeyState::Repeat;
			break;
		case GLFW_RELEASE:
			state = KeyState::Released;
			break;
		default:
			ASSERT(0);
		}
		m_Input->OnKeyInput(key, state);
	}
}