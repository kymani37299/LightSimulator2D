#pragma once

#define IM_BEGIN(X) ImGui::Begin(X, &m_Visible)
#define IM_END()	ImGui::End()

class UIElement
{
public:
	virtual ~UIElement() {}
	virtual void Init() = 0;
	virtual void Update(float dt) = 0;
	virtual void Render() = 0;

	inline void Show() { m_Visible = true; }
	inline bool IsVisible() const { return m_Visible; }

protected:
	bool m_Visible;
};