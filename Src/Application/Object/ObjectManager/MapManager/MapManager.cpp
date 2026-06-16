#include "MapManager.h"
#include "../ObjectManager.h"
#include "../../Ground/Ground.h"
#include "../../Building/Building.h"
#include "../../EventObject/DeliveryPoint.h"

// -------------------------------------------------------
// 初期化
// -------------------------------------------------------
void MapManager::Init() {}

// -------------------------------------------------------
// マップ全体の生成
// -------------------------------------------------------
std::vector<int> MapManager::GenarateMap(ObjectManager& objManager)
{
	GenerateRandomWalk();
	PlaceEventPoints(objManager, 3, 1);

	std::random_device rd;
	std::mt19937 gen(rd());

	// -------------------------------------------------------
	// 道パーツの接続フラグ
	// -------------------------------------------------------
	struct RoadConnect
	{
		bool px, mx, pz, mz; // 接続方向: +X, -X, +Z, -Z
		int count() const { return px + mx + pz + mz; }
	};

	// 指定座標が「道」かどうかを判定する
	auto isRoad = [&](int i, int j) -> bool
		{
			if (i < 0 || i >= MAP_WIDTH || j < 0 || j >= MAP_HEIGHT) return false;
			return mapType[j * MAP_WIDTH + i] == TILE_ROAD
				|| mapType[j * MAP_WIDTH + i] == TILE_START;
		};

	auto getRoadConnect = [&](int i, int j) -> RoadConnect
		{
			return {
				isRoad(i + 1, j),   // +X
				isRoad(i - 1, j),   // -X
				isRoad(i, j + 1),   // +Z
				isRoad(i, j - 1),   // -Z
			};
		};

	// -------------------------------------------------------
	// 道パーツ種別
	// -------------------------------------------------------
	enum class RoadType { Straight, Curve, Junction, Cross, End, None };

	// 接続情報から道パーツ種別と回転角度を返す
	// ベース定義:
	//   Straight  : +X/-X 方向（0°）
	//   Curve     : -X/-Z コーナー（0°）
	//   Junction  : +X/-X/-Z（+Z 欠け, 0°）
	auto getRoadTypeAndDir = [](const RoadConnect& c) -> std::pair<RoadType, float>
		{
			const int n = c.count();

			if (n == 4)
				return { RoadType::Cross, 0.0f };

			if (n == 3)
			{
				if (!c.pz) return { RoadType::Junction,   0.0f };
				if (!c.mz) return { RoadType::Junction, 180.0f };
				if (!c.mx) return { RoadType::Junction, 270.0f };
				if (!c.px) return { RoadType::Junction,  90.0f };
			}

			if (n == 2)
			{
				// 直線（対面接続）
				if (c.px && c.mx) return { RoadType::Straight,  0.0f };
				if (c.pz && c.mz) return { RoadType::Straight, 90.0f };

				// カーブ（隣接接続）
				if (c.mx && c.mz) return { RoadType::Curve,   0.0f };
				if (c.px && c.mz) return { RoadType::Curve, 270.0f };
				if (c.px && c.pz) return { RoadType::Curve, 180.0f };
				if (c.mx && c.pz) return { RoadType::Curve,  90.0f };
			}

			if (n == 1)
			{
				// 行き止まり
				if (c.pz) return { RoadType::End, 180.0f };
				if (c.mz) return { RoadType::End,   0.0f };
				if (c.mx) return { RoadType::End,  90.0f };
				if (c.px) return { RoadType::End, 270.0f };
			}

			return { RoadType::None, 0.0f };
		};

	// 道バリアント番号のマッピング
	auto roadVariantOf = [](RoadType type) -> int
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
		};

	// -------------------------------------------------------
	// 建物の向き計算用: 隣接する道の方向リスト
	// -------------------------------------------------------
	struct DirInfo { int dx, dy; float angle; };
	static constexpr DirInfo kDirections[4] = {
		{  0, -1,   0.0f },
		{ -1,  0,  90.0f },
		{  0,  1, 180.0f },
		{  1,  0, 270.0f },
	};

	std::vector<int> result;
	result.resize(mapType.size());

	// -------------------------------------------------------
	// タイル配置ループ
	// -------------------------------------------------------
	for (int i = 0; i < MAP_WIDTH; ++i)
	{
		for (int j = 0; j < MAP_HEIGHT; ++j)
		{
			const int idx = j * MAP_WIDTH + i;
			const int tile = mapType[idx];
			const Math::Vector3 pos(
				(i - MAP_WIDTH / 2.0f) * TILE_W,
				0.0f,
				(j - MAP_HEIGHT / 2.0f) * TILE_H
			);

			// ── 道タイル ──────────────────────────────────
			if (tile == TILE_ROAD || tile == TILE_START)
			{
				const auto [roadType, roadDir] = getRoadTypeAndDir(getRoadConnect(i, j));
				if (roadType == RoadType::None)
				{
					objManager.CreateObject<Ground>(pos, 0.0f, 0);
					mapType[idx] = 1;
				}
				else
				{
					objManager.CreateObject<Ground>(pos, roadDir, roadVariantOf(roadType));
				}
				continue;
			}

			// ── 外壁 ──────────────────────────────────────
			if (tile == TILE_WALL)
			{
				objManager.CreateObject<Ground>(pos, 0.0f, 0);
				objManager.CreateObject<Building>(pos + Math::Vector3(0, 0.1f, 0), 99, 0, 0.0f);
				continue;
			}

			// ── 空地（建物なし） ──────────────────────────
			if (tile <= 0 || tile >= 1)
			{
				objManager.CreateObject<Ground>(pos, 0.0f, 0);
				//continue;
			}

			// ── 建物タイル ────────────────────────────────
			{
				// 隣接する道の方向を収集し、ランダムに正面方向を決定する
				std::vector<float> roadDirs;
				for (const auto& d : kDirections)
				{
					const int ni = i + d.dx;
					const int nj = j + d.dy;
					if (ni >= 0 && ni < MAP_WIDTH && nj >= 0 && nj < MAP_HEIGHT)
					{
						if (mapType[nj * MAP_WIDTH + ni] == TILE_ROAD)
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

				// イベント地点には DeliveryPoint を正面にオフセットして配置
				if (tile == TILE_EVENT)
				{
					const float rad = dir * (M_PI / 180.0f);
					const Math::Vector3 frontOffset(
						-std::sin(rad) * 0.4f,
						0.0f,
						-std::cos(rad) * 0.4f
					);
					objManager.CreateObject<DeliveryPoint>(building, frontOffset, 0.4f);
				}
			}

			result[idx] = mapType[idx];
		}
	}
	return result;
}

// -------------------------------------------------------
// イベント地点をゾーン分割で配置
// -------------------------------------------------------
void MapManager::PlaceEventPoints(ObjectManager& /*objManager*/, int zoneDiv, int pointsPerZone)
{
	std::random_device rd;
	std::mt19937 gen(rd());

	// 外壁 1 マスを除いた有効範囲
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

			// ゾーン内の道タイルを列挙
			std::vector<std::pair<int, int>> roadTiles;
			for (int y = yStart; y <= yEnd; ++y)
			{
				for (int x = xStart; x <= xEnd; ++x)
				{
					if (mapType[y * MAP_WIDTH + x] <= 1)
						roadTiles.push_back({ x, y });
				}
			}

			if (roadTiles.empty()) continue;

			std::shuffle(roadTiles.begin(), roadTiles.end(), gen);
			const int placeCount = std::min(pointsPerZone, static_cast<int>(roadTiles.size()));

			for (int k = 0; k < placeCount; ++k)
			{
				const auto [x, y] = roadTiles[k];
				const Math::Vector3 pos(
					(x - MAP_WIDTH / 2.0f) * TILE_W,
					0.1f,
					(y - MAP_HEIGHT / 2.0f) * TILE_H
				);
				mapType[y * MAP_WIDTH + x] = TILE_EVENT;
				m_eventPoints.push_back(pos);
			}
		}
	}
}

// -------------------------------------------------------
// 2x2 の道ブロック形成チェック
// （(x,y) を道にすると 2x2 道エリアが生まれるか）
// -------------------------------------------------------
bool MapManager::WouldForm2x2(int x, int y)
{
	// (x,y) を含む可能性のある 2x2 の左上コーナー4候補をチェック
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
				// 現在のセル以外が道でなければ 2x2 にならない
				if (!isCurrent && mapType[ny * MAP_WIDTH + nx] > 0)
					allRoad = false;
			}
		}
		if (allRoad) return true;
	}
	return false;
}

// -------------------------------------------------------
// 移動可能チェック
// （壁・2x2 形成・隣接道数の上限を判定）
// -------------------------------------------------------
bool MapManager::IsValidMove(int x, int y)
{
	if (mapType[y * MAP_WIDTH + x] < 0) return false;
	if (WouldForm2x2(x, y))             return false;

	// 8 近傍に道が多すぎる場合は移動不可（迷路の密度制御）
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

// -------------------------------------------------------
// フロンティア展開によるランダムマップ生成
// -------------------------------------------------------
void MapManager::GenerateRandomWalk()
{
	// 全タイルを建物で初期化
	std::fill(mapType.begin(), mapType.end(), 1);

	// 開始点と外壁を設定
	constexpr int START_X = 15, START_Y = 15;
	mapType[START_Y * MAP_WIDTH + START_X] = TILE_START;

	for (int i = 0; i < MAP_WIDTH; ++i)
		for (int j = 0; j < MAP_HEIGHT; ++j)
			if (i == 0 || i == MAP_WIDTH - 1 || j == 0 || j == MAP_HEIGHT - 1)
				mapType[j * MAP_WIDTH + i] = TILE_WALL;

	// フロンティアキューで道を展開
	std::vector<std::pair<int, int>> frontier = { { START_X, START_Y } };

	std::random_device rd;
	std::mt19937 gen(rd());

	constexpr int dx[] = { 1, -1, 0,  0 };
	constexpr int dy[] = { 0,  0, 1, -1 };

	while (!frontier.empty())
	{
		std::uniform_int_distribution<> dist(0, static_cast<int>(frontier.size()) - 1);
		const int idx = dist(gen);
		const auto [cx, cy] = frontier[idx];

		// 展開可能な方向を収集
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
			// 展開できないフロンティアを除去
			frontier[idx] = frontier.back();
			frontier.pop_back();
		}
	}

	// ── ループ生成（事後的に壁を一部破壊してループを作る）──
	std::uniform_real_distribution<float> prob(0.0f, 1.0f);
	for (int y = 1; y < MAP_HEIGHT - 1; ++y)
	{
		for (int x = 1; x < MAP_WIDTH - 1; ++x)
		{
			if (mapType[y * MAP_WIDTH + x] != 1) continue;

			const bool horizontal = (mapType[y * MAP_WIDTH + (x - 1)] <= 0
				&& mapType[y * MAP_WIDTH + (x + 1)] <= 0);
			const bool vertical = (mapType[(y - 1) * MAP_WIDTH + x] <= 0
				&& mapType[(y + 1) * MAP_WIDTH + x] <= 0);

			if ((horizontal || vertical) && prob(gen) < 0.15f && !WouldForm2x2(x, y))
				mapType[y * MAP_WIDTH + x] = TILE_ROAD;
		}
	}
}