#pragma once

class UIObject;

enum class UIPaturn
{
	Title,
	Game,
	Result,
};

class UIManager
{
public :
	UIManager() {};
	~UIManager() {};

	void Init();
	void Update(float dt);
	void DrawSprite();

	void CreateUI(UIPaturn paturn);
private:
	std::vector<std::shared_ptr<UIObject>> m_UIobjList;
};