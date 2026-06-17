#include "SpawnManager.h"
#include "../ObjectManager.h"
#include "../../EventObject/SpeedUpObject/SpeedUpObject.h"
#include <algorithm>
#include <random>

SpawnManager::SpawnManager(std::unique_ptr<ObjectManager>& objManager)
{
	m_objManager = objManager.get();
}

void SpawnManager::Init()
{
	m_itemSpawnCnt = 3.f;
	m_spItemSub = GLOBALEVENT.subscribe<Events::Player::GetSpeedUp>([this](const Events::Player::GetSpeedUp& e)
		{
			Math::Vector3 pos = e.m_me.lock()->GetPos();
			int x = pos.x / TILE_W + MAP_WIDTH * 0.5f;
			int y = pos.z / TILE_H + MAP_HEIGHT * 0.5f;
			m_spawnSpItem[y * MAP_WIDTH + x] = 0;
			m_SpItemNum--;
		});
}

void SpawnManager::SetMapData(std::vector<int> mapData)
{
	m_mapData = mapData;
	m_spawnSpItem.resize(m_mapData.size());
	m_SpItemNum = 0;
	for (auto& data : m_spawnSpItem)
	{
		data = 0;
	}
	SpawnSpeedUp(10);
};

void SpawnManager::Update(float dt)
{
	m_itemSpawnCnt -= dt;
	if (m_itemSpawnCnt <= 0)
	{
		m_itemSpawnCnt = 3.f;
		SpawnSpeedUp(3);
	}
}

void SpawnManager::SpawnSpeedUp(int i)
{
	if (i <= 0) return;

	if (!m_objManager)return;

	// ① mapData から値が 1 のインデックスを収集
	std::vector<int> candidates;
	for (int idx = 0; idx < static_cast<int>(m_mapData.size()); ++idx)
	{
		if (m_mapData[idx] == 0 && m_spawnSpItem[idx] == 0 &&
			m_SpItemNum < 100)
		{
			candidates.push_back(idx);
		}
	}

	if (candidates.empty()) return;

	// ② シャッフルして均等にばらけさせる
	std::mt19937 rng(std::random_device{}());
	std::shuffle(candidates.begin(), candidates.end(), rng);

	// ③ 先頭から i 個（候補数を超えない範囲）取り出してスポーン
	int spawnCount = std::min(i, static_cast<int>(candidates.size()));
	for (int n = 0; n < spawnCount; ++n)
	{
		int idx = candidates[n];
		int col = idx % MAP_WIDTH;
		int row = idx / MAP_WIDTH;

		Math::Vector3 pos(
			(col - MAP_WIDTH * 0.5f) * TILE_W,
			0.2f,
			(row - MAP_HEIGHT * 0.5f) * TILE_H
		);

		m_objManager->CreateObject<SpeedUpObject>(pos);
		m_spawnSpItem[idx] = 1;
		m_SpItemNum++;

	}
}