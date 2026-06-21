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
public:
	static UIManager& Instance()
	{
		static UIManager instance;
		return instance;
	}

	void Init();
	void Update(float dt);
	void DrawSprite();
	void Release();

	void CreateUI(UIPaturn paturn);
private:
	std::vector<std::shared_ptr<UIObject>> m_UIobjList;

	ScopedSubscriber m_titleSub;
	ScopedSubscriber m_gameSub;
	ScopedSubscriber m_resultSub;

private:
	UIManager() {};
	~UIManager() { Release(); };
};