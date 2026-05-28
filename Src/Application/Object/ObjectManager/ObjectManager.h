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
	void CreateObject(ObjectType type);

private:
	std::vector<KdGameObject*> m_objList;
};