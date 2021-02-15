#pragma once

#include <vector>

class Window;
class UIElement;

class UIEngine
{
public:
	~UIEngine();

	void Init(Window* window);
	void Update(float dt);
	void Render();
	
private:
	void BeginFrame();
	void EndFrame();

	void SetupElements();

	void AddElement(UIElement* e);
	void RemoveElement(size_t index);

private:
	std::vector<UIElement*> m_Elements;
};