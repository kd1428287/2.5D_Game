#include "MapManager.h"
#include "../ObjectManager.h"
#include "../../Ground/Ground.h"
#include "../../Building/Building.h"
#include "../../EventObject/DeliveryPoint.h"

void MapManager::Init() {}

void MapManager::GenarateMap(ObjectManager& _objManager)
{
	// 1. 道路生成（近傍チェック付きランダムウォーク）
	GenerateRandomWalk(2000);

	// 2. データに基づきオブジェクトを配置
	float width = 0.8f;
	float height = 0.8f;

	for (int i = 0; i < MAP_WIDTH; i++) {
		for (int j = 0; j < MAP_HEIGHT; j++) {
			Math::Vector3 pos((i - MAP_WIDTH / 2.0f) * width, 0, (j - MAP_HEIGHT / 2.0f) * height);
			int idx = j * MAP_WIDTH + i;

			if (mapType[idx] == 0)
			{
				_objManager.CreateObject<Ground>(pos, 0, 1);
			}
			else
			{
				_objManager.CreateObject<Ground>(pos, 0, 0);
			}

			if (i == 0 || i == MAP_WIDTH - 1 || j == 0 || j == MAP_HEIGHT - 1) {
				_objManager.CreateObject<Building>(pos + Math::Vector3(0, 0.1f, 0), 6);
				continue;
			}

			if (mapType[idx] != 0) {
				_objManager.CreateObject<Building>(pos + Math::Vector3(0, 0.1f, 0), (int)(i % 6));
			}
		}
	}
}

bool MapManager::IsValidMove(int x, int y) {
	int count = 0;
	// 8近傍チェック
	for (int dy = -1; dy <= 1; ++dy) {
		for (int dx = -1; dx <= 1; ++dx) {
			if (dx == 0 && dy == 0) continue;
			int nx = x + dx, ny = y + dy;
			if (nx > 0 && nx < MAP_WIDTH - 1 && ny > 0 && ny < MAP_HEIGHT - 1) {
				if (mapType[ny * MAP_WIDTH + nx] == 0) count++;
			}
		}
	}
	// 既存の道なら例外的に許容(交差用)、そうでなければ隣接道路が1つ以下ならOK
	return (mapType[y * MAP_WIDTH + x] == 0) || (count <= 2);
}

void MapManager::GenerateRandomWalk(int steps)
{
	std::fill(mapType.begin(), mapType.end(), 1);

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dist(0, 3);
	std::uniform_int_distribution<> xDist(1, MAP_WIDTH - 2);
	std::uniform_int_distribution<> yDist(1, MAP_HEIGHT - 2);

	int dx[] = { 1, -1, 0, 0 };
	int dy[] = { 0, 0, 1, -1 };

	// 密度を上げるため、試行回数を増やし、テレポートを導入する
	int agents = 10; // エージェント数
	for (int s = 0; s < agents; ++s) {
		int curX = xDist(gen);
		int curY = yDist(gen);
		mapType[curY * MAP_WIDTH + curX] = 0;

		for (int i = 0; i < steps / agents; ++i) {
			int dir = dist(gen);
			int nx = curX + dx[dir];
			int ny = curY + dy[dir];

			if (nx > 0 && nx < MAP_WIDTH - 1 && ny > 0 && ny < MAP_HEIGHT - 1) {
				if (IsValidMove(nx, ny)) {
					curX = nx;
					curY = ny;
					mapType[curY * MAP_WIDTH + curX] = 0;
				}
				else {
					// 【重要】動けない場合、近くの「壁」の場所へテレポートして探索を継続
					curX = xDist(gen);
					curY = yDist(gen);
				}
			}
		}
	}
}