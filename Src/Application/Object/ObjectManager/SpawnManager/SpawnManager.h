#pragma once

class ObjectManager;

class SpawnManager
{
public:
	//SpawnManager() {};
	SpawnManager(std::unique_ptr<ObjectManager>& objManager);
	~SpawnManager() {};

	void Init();
	void SetMapData(std::vector<int> mapData);
	void Update(float dt);

	void SpawnSpeedUp(int i);

private:
	// -------------------------------------------------------
	// 定数
	// -------------------------------------------------------
	static constexpr int MAP_WIDTH = 30;
	static constexpr int MAP_HEIGHT = 30;

	// タイル幅・高さ（ワールド空間）
	static constexpr float TILE_W = 0.8f;
	static constexpr float TILE_H = 0.8f;

	// mapType の特殊値
	static constexpr int TILE_ROAD = 0;		// 道
	static constexpr int TILE_START = -1;   // 開始点（道扱い）
	static constexpr int TILE_HOME = -2;	// 開始点
	static constexpr int TILE_EVENT = -3;   // イベント地点
	static constexpr int TILE_WALL = -9;	// 外壁

	std::vector<int> m_mapData;
	std::vector<int> m_spawnSpItem;
	int m_SpItemNum = 0;
	//std::unique_ptr<ObjectManager>& m_objManagaer;
	ObjectManager* m_objManager = nullptr;

	float m_itemSpawnCnt = 0.f;

	ScopedSubscriber m_spItemSub;
};