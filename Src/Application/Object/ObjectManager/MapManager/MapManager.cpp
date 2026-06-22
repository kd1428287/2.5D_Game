#include "MapManager.h"
#include "../ObjectManager.h"
#include "../../Ground/Ground.h"
#include "../../Building/Building.h"
#include "../../EventObject/DeliveryPoint.h"
#include "../../../System/Reader/Reader.h"
#include <random>
#include <algorithm>
#include <cmath>

// ── ファイル内共通定数 ──────────────────────────────────
namespace
{
	struct DirInfo { int dx, dy; float angle; };
	constexpr DirInfo kDirections[4] = {
		{  0, -1,   0.0f },
		{ -1,  0,  90.0f },
		{  0,  1, 180.0f },
		{  1,  0, 270.0f },
	};
}

void MapManager::Init() {}

bool MapManager::IsRoadTile(int t) const
{
	return t == TILE_ROAD || t == TILE_START;
}

// ── 道の接続・形状計算ヘルパー（共通化） ─────────────────────
MapManager::RoadConnect MapManager::GetRoadConnect(int i, int j, const int* mapData, int width, int height) const
{
	auto isRoad = [&](int ni, int nj) -> bool
		{
			if (ni < 0 || ni >= width || nj < 0 || nj >= height) return false;
			return IsRoadTile(mapData[nj * width + ni]);
		};
	return {
		isRoad(i + 1, j),
		isRoad(i - 1, j),
		isRoad(i, j + 1),
		isRoad(i, j - 1),
	};
}

std::pair<MapManager::RoadType, float> MapManager::GetRoadTypeAndDir(const RoadConnect& c) const
{
	const int n = c.count();
	if (n == 4) return { RoadType::Cross, 0.0f };
	if (n == 3)
	{
		if (!c.pz) return { RoadType::Junction,   0.0f };
		if (!c.mz) return { RoadType::Junction, 180.0f };
		if (!c.mx) return { RoadType::Junction, 270.0f };
		if (!c.px) return { RoadType::Junction,  90.0f };
	}
	if (n == 2)
	{
		if (c.px && c.mx) return { RoadType::Straight,  0.0f };
		if (c.pz && c.mz) return { RoadType::Straight, 90.0f };
		if (c.mx && c.mz) return { RoadType::Curve,    0.0f };
		if (c.px && c.mz) return { RoadType::Curve,  270.0f };
		if (c.px && c.pz) return { RoadType::Curve,  180.0f };
		if (c.mx && c.pz) return { RoadType::Curve,   90.0f };
	}
	if (n == 1)
	{
		if (c.pz) return { RoadType::End, 180.0f };
		if (c.mz) return { RoadType::End,   0.0f };
		if (c.mx) return { RoadType::End,  90.0f };
		if (c.px) return { RoadType::End, 270.0f };
	}
	return { RoadType::None, 0.0f };
}

int MapManager::GetRoadVariant(RoadType type) const
{
	switch (type)
	{
	case RoadType::Straight:  return 1;
	case RoadType::Curve:     return 2;
	case RoadType::Junction:  return 3;
	case RoadType::Cross:     return 4;
	case RoadType::End:       return 5;
	default:                  return 0;
	}
}

// ── タイル自動分類 ──────────────────────────────────────
void MapManager::ClassifyTiles()
{
	constexpr int NDX[4] = { 0,  0,  1, -1 };
	constexpr int NDY[4] = { -1,  1,  0,  0 };

	std::vector<int> toBuilding;
	for (int j = 0; j < MAP_HEIGHT; ++j)
	{
		for (int i = 0; i < MAP_WIDTH; ++i)
		{
			const int idx = j * MAP_WIDTH + i;
			const int tile = mapType[idx];

			if (tile < 1) continue;

			bool allNonPos = true;
			for (int d = 0; d < 4; ++d)
			{
				const int ni = i + NDX[d];
				const int nj = j + NDY[d];

				if (ni < 0 || ni >= MAP_WIDTH || nj < 0 || nj >= MAP_HEIGHT)
				{
					allNonPos = false;
					break;
				}

				if (mapType[nj * MAP_WIDTH + ni] > 0)
				{
					allNonPos = false;
					break;
				}
			}

			if (allNonPos)
				toBuilding.push_back(idx);
		}
	}

	for (int idx : toBuilding)
		mapType[idx] = TILE_BUILDING;
}

// ── メインマップのオブジェクト生成 ────────────────────────
std::vector<int> MapManager::GenerateMapForMapData(ObjectManager& objManager)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::vector<int> result(mapType.size());

	for (int j = 0; j < MAP_HEIGHT; ++j)
	{
		for (int i = 0; i < MAP_WIDTH; ++i)
		{
			const int idx = j * MAP_WIDTH + i;
			const int tile = mapType[idx];
			const Math::Vector3 pos(
				(i - MAP_WIDTH / 2.0f) * TILE_W,
				0.0f,
				(j - MAP_HEIGHT / 2.0f) * TILE_H
			);

			// 道タイル
			if (IsRoadTile(tile))
			{
				const auto [roadType, roadDir] = GetRoadTypeAndDir(GetRoadConnect(i, j, mapType.data(), MAP_WIDTH, MAP_HEIGHT));
				if (roadType == RoadType::None)
					objManager.CreateObject<Ground>(pos, 0.0f, 0);
				else
					objManager.CreateObject<Ground>(pos, roadDir, GetRoadVariant(roadType));

				result[idx] = tile;
				continue;
			}

			// 外壁
			if (tile == TILE_WALL)
			{
				objManager.CreateObject<Ground>(pos, 0.0f, 0);
				objManager.CreateObject<Building>(pos + Math::Vector3(0, 0.1f, 0), 99, 0, 0.0f);
				result[idx] = tile;
				continue;
			}

			// 建物タイル全般
			{
				objManager.CreateObject<Ground>(pos, 0.0f, 0);

				if (tile == TILE_HOME)
				{
					objManager.CreateObject<Building>(pos + Math::Vector3(0, 0.1f, 0), 10, 2, 0.0f);
					result[idx] = tile;
					continue;
				}

				if (tile == TILE_BUILDING)
				{
					objManager.CreateObject<Building>(pos + Math::Vector3(0, 0.1f, 0), 6, 0, 0.0f);
					result[idx] = tile;
					continue;
				}

				std::vector<float> roadDirs;
				for (const auto& d : kDirections)
				{
					const int ni = i + d.dx;
					const int nj = j + d.dy;
					if (ni >= 0 && ni < MAP_WIDTH && nj >= 0 && nj < MAP_HEIGHT)
					{
						if (IsRoadTile(mapType[nj * MAP_WIDTH + ni]))
							roadDirs.push_back(d.angle);
					}
				}

				float dir = 0.0f;
				if (!roadDirs.empty())
				{
					std::uniform_int_distribution<> dist(0, static_cast<int>(roadDirs.size()) - 1);
					dir = roadDirs[dist(gen)];
				}

				auto building = objManager.CreateObject<Building>(
					pos + Math::Vector3(0, 0.1f, 0),
					static_cast<int>(KdRandom::GetFloat(3.0f, 6.9f)),
					1,
					dir
				);

				if (tile == TILE_EVENT)
				{
					const float rad = dir * (M_PI / 180.0f);
					const Math::Vector3 frontOffset(-std::sin(rad) * 0.4f, 0.0f, -std::cos(rad) * 0.4f);
					objManager.CreateObject<DeliveryPoint>(building, frontOffset, 0.4f);
				}

				result[idx] = tile;
			}
		}
	}
	return result;
}

// ── リザルト画面用：決め打ちマップ生成（★大幅改善） ────────────────
void MapManager::GenerateResultMap(ObjectManager& objManager)
{
	// マップサイズ定義
	constexpr int RESULT_W = 10;
	constexpr int RESULT_H = 10;

	// 可読性のための定数エイリアス
	constexpr int G = 1;
	constexpr int R = TILE_ROAD;
	constexpr int B = TILE_BUILDING;
	constexpr int W = TILE_WALL;
	constexpr int H = 2;

	// リザルト用の決め打ち配置データ (タイル定数を直接使えるので編集が超簡単)
	const std::vector<int> resultMap =
	{
		H, H, H, H, H, H, H, H, H, H,
		H, H, H, H, H, H, H, H, H, H,
		H, H, H, H, H, H, H, H, H, H,
		H, H, H, H, H, H, H, H, H, H,
		H, H, H, H, H, H, H, H, H, H,
		H, H, H, H, H, H, H, H, H, H,
		H, H, H, H, H, H, R, R, R, R,
		H, H, H, H, H, H, R, G, G, G,
		H, H, H, H, H, H, R, G, G, G,
		H, H, H, H, H, H, R, G, G, G,
	};

	for (int j = 0; j < RESULT_H; ++j)
	{
		for (int i = 0; i < RESULT_W; ++i)
		{
			const int idx = j * RESULT_W + i;
			const int tile = resultMap[idx];

			// リザルトマップの中心を基準とした座標計算
			const Math::Vector3 pos(
				(i - RESULT_W / 2.0f) * TILE_W,
				0.0f,
				(j - RESULT_H / 2.0f) * TILE_H
			);

	/*		const Math::Vector3 pos(
				i * TILE_W,
				0.0f,
				j * TILE_H
			);*/

			// ── 1. 道タイルの配置 ─────────────────────────
			if (IsRoadTile(tile))
			{
				const auto [roadType, roadDir] = GetRoadTypeAndDir(GetRoadConnect(i, j, resultMap.data(), RESULT_W, RESULT_H));
				if (roadType != RoadType::None)
				{
					// 適切なパーツと回転で道オブジェクトを上書き生成
					objManager.CreateObject<Ground>(pos, roadDir, GetRoadVariant(roadType));
				}
				continue;
			}

			// すべてのマスのベースに地面(Ground)を敷く
			objManager.CreateObject<Ground>(pos, 0.0f, 0);

			// ── 3. 建物タイルの配置（TILE_BUILDING / TILE_HOME / 空き地） ──
			if (tile == TILE_BUILDING || tile == TILE_HOME || tile >= 2)
			{
				// 隣接する道を探して建物の正面の向きを決める
				std::vector<float> roadDirs;
				for (const auto& d : kDirections)
				{
					const int ni = i + d.dx;
					const int nj = j + d.dy;
					if (ni >= 0 && ni < RESULT_W && nj >= 0 && nj < RESULT_H)
					{
						if (IsRoadTile(resultMap[nj * RESULT_W + ni]))
							roadDirs.push_back(d.angle);
					}
				}

				// リザルトは固定画面なので、ランダムではなく最初に見つかった道を正面とする
				float dir = roadDirs.empty() ? 0.0f : roadDirs[0];

				// タイルに応じた建物パラメータの設定
				int type = 6;
				int variant = 0;
				if (tile == 2)
				{
					type = 0;
					variant = 1;
				}

				objManager.CreateObject<Building>(pos + Math::Vector3(0, 0.1f, 0), type, variant, dir);
			}
		}
	}
}

// ── 以下、既存の自動生成アルゴリズム（変更なし） ─────────────────────
void MapManager::PlaceEventPoints(ObjectManager&, int zoneDiv, int pointsPerZone)
{
	std::random_device rd;
	std::mt19937 gen(rd());

	constexpr int NDX[4] = { 0,  0,  1, -1 };
	constexpr int NDY[4] = { -1,  1,  0,  0 };

	const int validW = MAP_WIDTH - 2;
	const int validH = MAP_HEIGHT - 2;
	const int zoneW = validW / zoneDiv;
	const int zoneH = validH / zoneDiv;

	for (int zy = 0; zy < zoneDiv; ++zy)
	{
		for (int zx = 0; zx < zoneDiv; ++zx)
		{
			const int xStart = 1 + zx * zoneW;
			const int yStart = 1 + zy * zoneH;
			const int xEnd = (zx == zoneDiv - 1) ? MAP_WIDTH - 2 : xStart + zoneW - 1;
			const int yEnd = (zy == zoneDiv - 1) ? MAP_HEIGHT - 2 : yStart + zoneH - 1;

			std::vector<std::pair<int, int>> candidates;
			for (int y = yStart; y <= yEnd; ++y)
			{
				for (int x = xStart; x <= xEnd; ++x)
				{
					const int tile = mapType[y * MAP_WIDTH + x];
					if (tile < 1) continue;

					bool adjacentToRoad = false;
					for (int d = 0; d < 4; ++d)
					{
						const int ni = x + NDX[d];
						const int nj = y + NDY[d];
						if (ni >= 0 && ni < MAP_WIDTH && nj >= 0 && nj < MAP_HEIGHT)
						{
							if (IsRoadTile(mapType[nj * MAP_WIDTH + ni]))
							{
								adjacentToRoad = true;
								break;
							}
						}
					}

					if (adjacentToRoad)
						candidates.push_back({ x, y });
				}
			}

			if (candidates.empty()) continue;

			std::shuffle(candidates.begin(), candidates.end(), gen);
			const int placeCount = std::min(pointsPerZone, static_cast<int>(candidates.size()));

			for (int k = 0; k < placeCount; ++k)
			{
				const auto [x, y] = candidates[k];
				const Math::Vector3 pos((x - MAP_WIDTH / 2.0f) * TILE_W, 0.1f, (y - MAP_HEIGHT / 2.0f) * TILE_H);
				mapType[y * MAP_WIDTH + x] = TILE_EVENT;
				m_eventPoints.push_back(pos);
			}
		}
	}

	Reader::Instance().WriteScore({ 0, 0, static_cast<float>(m_eventPoints.size()) });
}

std::vector<int> MapManager::GenerateMap(ObjectManager& objManager)
{
	GenerateRandomWalk();
	ClassifyTiles();
	PlaceEventPoints(objManager, 3, 1);

	return GenerateMapForMapData(objManager);
}

bool MapManager::WouldForm2x2(int x, int y)
{
	const int corners[4][2] = {
		{ x - 1, y - 1 }, { x, y - 1 },
		{ x - 1, y     }, { x, y     },
	};

	for (const auto& c : corners)
	{
		const int lx = c[0], ly = c[1];
		if (lx < 0 || lx + 1 >= MAP_WIDTH || ly < 0 || ly + 1 >= MAP_HEIGHT) continue;

		bool allRoad = true;
		for (int dy = 0; dy <= 1 && allRoad; ++dy)
		{
			for (int dx = 0; dx <= 1 && allRoad; ++dx)
			{
				const int nx = lx + dx, ny = ly + dy;
				const bool isCurrent = (nx == x && ny == y);
				if (!isCurrent && mapType[ny * MAP_WIDTH + nx] > 0)
					allRoad = false;
			}
		}
		if (allRoad) return true;
	}
	return false;
}

bool MapManager::IsValidMove(int x, int y)
{
	if (mapType[y * MAP_WIDTH + x] < 0) return false;
	if (WouldForm2x2(x, y))             return false;

	int adjacentRoadCount = 0;
	for (int dy = -1; dy <= 1; ++dy)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			if (dx == 0 && dy == 0) continue;
			const int nx = x + dx, ny = y + dy;
			if (nx > 0 && nx < MAP_WIDTH - 1 && ny > 0 && ny < MAP_HEIGHT - 1)
			{
				if (mapType[ny * MAP_WIDTH + nx] <= 0)
					++adjacentRoadCount;
			}
		}
	}
	return adjacentRoadCount <= 2;
}

void MapManager::GenerateRandomWalk()
{
	std::fill(mapType.begin(), mapType.end(), 1);

	constexpr int START_X = MAP_WIDTH / 2;
	constexpr int START_Y = MAP_HEIGHT / 2;
	mapType[START_Y * MAP_WIDTH + START_X] = TILE_START;
	mapType[(START_Y - 1) * MAP_WIDTH + START_X] = TILE_HOME;

	for (int i = 0; i < MAP_WIDTH; ++i)
		for (int j = 0; j < MAP_HEIGHT; ++j)
			if (i == 0 || i == MAP_WIDTH - 1 || j == 0 || j == MAP_HEIGHT - 1)
				mapType[j * MAP_WIDTH + i] = TILE_WALL;

	std::vector<std::pair<int, int>> frontier = { { START_X, START_Y } };

	std::random_device rd;
	std::mt19937 gen(rd());

	constexpr int dx[] = { 1, -1,  0,  0 };
	constexpr int dy[] = { 0,  0,  1, -1 };

	while (!frontier.empty())
	{
		std::uniform_int_distribution<> dist(0, static_cast<int>(frontier.size()) - 1);
		const int idx = dist(gen);
		const auto [cx, cy] = frontier[idx];

		std::vector<int> validDirs;
		for (int i = 0; i < 4; ++i)
		{
			const int nx = cx + dx[i], ny = cy + dy[i];
			if (nx > 0 && nx < MAP_WIDTH - 1 && ny > 0 && ny < MAP_HEIGHT - 1)
			{
				if (mapType[ny * MAP_WIDTH + nx] == 1 && IsValidMove(nx, ny))
					validDirs.push_back(i);
			}
		}

		if (!validDirs.empty())
		{
			std::uniform_int_distribution<> dirDist(0, static_cast<int>(validDirs.size()) - 1);
			const int d = validDirs[dirDist(gen)];
			const int nx = cx + dx[d], ny = cy + dy[d];
			mapType[ny * MAP_WIDTH + nx] = TILE_ROAD;
			frontier.push_back({ nx, ny });
		}
		else
		{
			frontier[idx] = frontier.back();
			frontier.pop_back();
		}
	}

	std::uniform_real_distribution<float> prob(0.0f, 1.0f);
	for (int y = 1; y < MAP_HEIGHT - 1; ++y)
	{
		for (int x = 1; x < MAP_WIDTH - 1; ++x)
		{
			if (mapType[y * MAP_WIDTH + x] != 1) continue;

			const bool horizontal = (mapType[y * MAP_WIDTH + (x - 1)] <= 0 && mapType[y * MAP_WIDTH + (x + 1)] <= 0);
			const bool vertical = (mapType[(y - 1) * MAP_WIDTH + x] <= 0 && mapType[(y + 1) * MAP_WIDTH + x] <= 0);

			if ((horizontal || vertical) && prob(gen) < 0.15f && !WouldForm2x2(x, y))
				mapType[y * MAP_WIDTH + x] = TILE_ROAD;
		}
	}
}