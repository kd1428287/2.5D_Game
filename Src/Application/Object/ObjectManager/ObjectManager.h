#pragma once

enum class ObjectType
{
	player,
	ground,
	building,
};

class ObjectManager
{
public:
	ObjectManager() {};
	~ObjectManager() {};

	void Init();
	void PreUpdate();
	void Update(float dt);
	void PostUpdate();

	void GenerateDepthMapFromLight();
	void PreDraw();
	void DrawLit();
	void DrawUnLit();
	void DrawEffect();
	void DrawBright();
	void DrawSprite();
	void DrawDebug();

	const std::vector<std::shared_ptr<KdGameObject>>& GetObjList() 
	{
		return m_objList;
	}

	void AddObject(const std::shared_ptr<KdGameObject>& obj)
	{
		m_nextObjList.push_back(obj);
	}

	template<typename T,typename... Args>
	std::shared_ptr<T> CreateObject(Args ... args)
	{
		std::shared_ptr<T> obj = std::make_shared<T>(args ...);
		obj->Init();
		m_objList.push_back(obj);
		return obj;
	}

private:
	std::vector<std::shared_ptr<KdGameObject>> m_objList;
	std::vector<std::shared_ptr<KdGameObject>> m_nextObjList;
};
