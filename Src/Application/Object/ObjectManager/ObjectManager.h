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

	template<typename T>
	T* CreateObject(Math::Vector3 pos)
	{
		std::shared_ptr<T> obj = std::make_shared<T>();
		obj->Init();
		m_objList.push_back(obj);
		return obj.get();
	}

private:
	std::vector<std::shared_ptr<KdGameObject>> m_objList;
};
