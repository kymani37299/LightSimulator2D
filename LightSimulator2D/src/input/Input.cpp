#include "Input.h"

#include "core/Window.h"

static inline void ProcessInputMap(InputMap& map, KeyInput key, KeyState state)
{
	if (map.Contains(key))
	{
		for (KeyCallback* callback : map[key])
		{
			(*callback)();
			(*callback)(state);
		}
	}
}

// Mouse position in (-1,-1) to (1,1) space
// (-1 -1) is down left
Vec2 Input::GetMousePosition()
{
	Vec2 absolutePosition = m_Window->GetMousePosition();
	absolutePosition /= Vec2(SCREEN_WIDTH, SCREEN_HEIGHT);
	absolutePosition *=  2.0;
	absolutePosition -= 1.0;
	absolutePosition.y *= -1.0;
	return absolutePosition;
}

void Input::AddKeyInputCallback(KeyInput key, KeyCallback* callback)
{
	m_KeyInputCallbacks[key].push_back(callback);
}

void Input::AddKeyPressedCallback(KeyInput key, KeyCallback* callback)
{
	m_KeyPressedCallbacks[key].push_back(callback);
}

void Input::AddStateChangedCallback(KeyInput key, KeyCallback* callback)
{
	m_StateChangedCallback[key].push_back(callback);
}

void Input::OnKeyInput(KeyInput key, KeyState state)
{
	if (state != KeyState::Repeat)
	{
		ProcessInputMap(m_StateChangedCallback, key, state);
	}

	if (state == KeyState::Pressed)
	{
		ProcessInputMap(m_KeyPressedCallbacks, key, state);
	}

	if (state == KeyState::Pressed || state == KeyState::Repeat)
	{
		ProcessInputMap(m_KeyInputCallbacks, key, state);
	}
}