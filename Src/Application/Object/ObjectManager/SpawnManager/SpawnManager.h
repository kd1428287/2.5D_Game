#pragma once

class ObjectManager;

class SpawnManager
{
public:
	SpawnManager() {};
	SpawnManager(std::shared_ptr<ObjectManager> objManagaer) { m_objManagaer = objManagaer; };
	~SpawnManager() {};

	void SetMapData(std::vector<int> mapData) { m_mapData = mapData; };
	void Update(float dt);

private:
	std::vector<int> m_mapData;
	std::weak_ptr<ObjectManager> m_objManagaer;
};