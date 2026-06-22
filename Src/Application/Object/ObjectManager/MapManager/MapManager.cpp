#include "MapManager.h"
#include "../ObjectManager.h"
#include "../../Ground/Ground.h"
#include "../../Building/Building.h"
#include "../../EventObject/DeliveryPoint.h"
#include "../../../System/Reader/Reader.h"

// -------------------------------------------------------
// 初期化
// -------------------------------------------------------
void MapManager::Init() {}

// -------------------------------------------------------
// 道タイル判定ヘルパー（重複定義を排除）
//
// ROAD / START は「プレイヤーが走る道」。
// EVENT は建物タイルに付与される特殊値なので道扱いしない。
// -------------------------------------------------------
bool MapManager::IsRoadTile(int t) const
{
	return t == TILE_ROAD || t == TILE_START;
}

// -------------------------------------------------------
// タイプ分け確定フェーズ
//
// 呼び出しタイミング: GenerateRandomWalk() の直後。
//                     PlaceEventPoints() より前に実行すること。
//
// 変換ルール:
//   空き地タイル（>= 1）で、上下左右 4 方向がすべて非正数の場合
//   → TILE_BUILDING（孤立した空き地を建物で埋める）
//
// ※ TILE_WALL(-9) および既確定の建物タイルは変換対象外。
// -------------------------------------------------------
void MapManager::ClassifyTiles()
{
	constexpr int NDX[4] = { 0,  0,  1, -1 };
	constexpr int NDY[4] = { -1,  1,  0,  0 };

	// mapType を走査中に書き換えないよう、変換対象を収集してから一括適用する
	std::vector<int> toBuilding;

	for (int j = 0; j < MAP_HEIGHT; ++j)
	{
		for (int i = 0; i < MAP_WIDTH; ++i)
		{
			const int idx = j * MAP_WIDTH + i;
			const int tile = mapType[idx];

			// 外壁・道タイル・特殊タイル（負値）は対象外
			// 空き地候補は tile >= 1 のみ
			if (tile < 1) continue;

			bool allNonPos = true;
			for (int d = 0; d < 4; ++d)
			{
				const int ni = i + NDX[d];
				const int nj = j + NDY[d];

				if (ni < 0 || ni >= MAP_WIDTH || nj < 0 || nj >= MAP_HEIGHT)
				{
					// マップ外は「非正数ではない」として扱う（境界を建物化しない）
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

// -------------------------------------------------------
// イベント地点をゾーン分割で配置
//
// 呼び出しタイミング: ClassifyTiles() の後。
//                     タイル確定済みの mapType を参照するため、
//                     必ず ClassifyTiles() より後に呼ぶこと。
//
// 候補条件:
//   ・タイル値が >= 1（空き地：建物になる予定だが未配置）
//   ・かつ、上下左右 4 方向のいずれかに道タイル（ROAD / START）が隣接している
//
// 選ばれたタイルは TILE_EVENT に変更され、GenarateMap() の
// オブジェクト生成ループで DeliveryPoint 付き建物として生成される。
// -------------------------------------------------------
void MapManager::PlaceEventPoints(ObjectManager& /*objManager*/, int zoneDiv, int pointsPerZone)
{
	std::random_device rd;
	std::mt19937 gen(rd());

	constexpr int NDX[4] = { 0,  0,  1, -1 };
	constexpr int NDY[4] = { -1,  1,  0,  0 };

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

			// ゾーン内で「道に隣接した空き地タイル（>= 1）」を列挙
			// ※ TILE_BUILDING(-7) は ClassifyTiles() で孤立確定済みのため除外
			std::vector<std::pair<int, int>> candidates;
			for (int y = yStart; y <= yEnd; ++y)
			{
				for (int x = xStart; x <= xEnd; ++x)
				{
					const int tile = mapType[y * MAP_WIDTH + x];
					if (tile < 1) continue; // 道・壁・特殊タイルは除外

					// 四方いずれかに道タイルがあれば候補とする
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

	Reader::Instance().WriteScore({ 0, 0, static_cast<float>(m_eventPoints.size()) });
}

// -------------------------------------------------------
// マップ全体の生成
//
// フロー:
//   1. GenerateRandomWalk()  … 道・空き地の基本配置
//   2. ClassifyTiles()       … 孤立タイルを TILE_BUILDING に確定
//   3. PlaceEventPoints()    … 道に隣接した空き地を TILE_EVENT に設定
//                              （★タイル確定後に行うことで配置品質を保証）
//   4. タイル配置ループ       … 確定した mapType を元にオブジェクト生成
// -------------------------------------------------------
std::vector<int> MapManager::GenerateMap(ObjectManager& objManager)
{
	// ── フェーズ 1〜3: タイプ分け確定 ───────────────────────
	GenerateRandomWalk();
	ClassifyTiles();                        // ★先にタイルを確定
	PlaceEventPoints(objManager, 3, 1);    // ★確定後にイベント配置

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

	auto getRoadConnect = [&](int i, int j) -> RoadConnect
		{
			auto isRoad = [&](int ni, int nj) -> bool
				{
					if (ni < 0 || ni >= MAP_WIDTH || nj < 0 || nj >= MAP_HEIGHT) return false;
					return IsRoadTile(mapType[nj * MAP_WIDTH + ni]);
				};
			return {
				isRoad(i + 1, j),
				isRoad(i - 1, j),
				isRoad(i, j + 1),
				isRoad(i, j - 1),
			};
		};

	// -------------------------------------------------------
	// 道パーツ種別
	// -------------------------------------------------------
	enum class RoadType { Straight, Curve, Junction, Cross, End, None };

	auto getRoadTypeAndDir = [](const RoadConnect& c) -> std::pair<RoadType, float>
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
		};

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

	std::vector<int> result(mapType.size());

	// ── フェーズ 4: オブジェクト生成 ────────────────────────
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

			// ── 道タイル ──────────────────────────────────────
			if (IsRoadTile(tile))
			{
				const auto [roadType, roadDir] = getRoadTypeAndDir(getRoadConnect(i, j));
				if (roadType == RoadType::None)
					objManager.CreateObject<Ground>(pos, 0.0f, 0);
				else
					objManager.CreateObject<Ground>(pos, roadDir, roadVariantOf(roadType));

				result[idx] = tile;
				continue;
			}

			// ── 外壁 ──────────────────────────────────────────
			if (tile == TILE_WALL)
			{
				objManager.CreateObject<Ground>(pos, 0.0f, 0);
				objManager.CreateObject<Building>(pos + Math::Vector3(0, 0.1f, 0), 99, 0, 0.0f);
				result[idx] = tile;
				continue;
			}

			// ── 建物タイル（空き地 >= 1、TILE_BUILDING、TILE_EVENT） ──
			// Ground を敷いてから建物を生成する
			{
				objManager.CreateObject<Ground>(pos, 0.0f, 0);

				if (tile == TILE_HOME)
				{
					objManager.CreateObject<Building>(
						pos + Math::Vector3(0, 0.1f, 0),
						10, 2, 0.0f
					);
					result[idx] = tile;
					continue;
				}

				if (tile == TILE_BUILDING)
				{
					objManager.CreateObject<Building>(
						pos + Math::Vector3(0, 0.1f, 0),
						6, 0, 0.0f
					);
					result[idx] = tile;
					continue;
				}

				// 隣接する道方向を収集してランダムに正面を決定
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

				// TILE_EVENT のみ DeliveryPoint を正面にオフセットして配置
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

				result[idx] = tile;
			}
		}
	}

	return result;
}

// -------------------------------------------------------
// 2x2 の道ブロック形成チェック
// （(x,y) を道にすると 2x2 道エリアが生まれるか）
// -------------------------------------------------------
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

// -------------------------------------------------------
// 移動可能チェック
// （壁・2x2 形成・隣接道数の上限を判定）
// -------------------------------------------------------
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

// -------------------------------------------------------
// フロンティア展開によるランダムマップ生成
// -------------------------------------------------------
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

	// ループ生成（事後的に壁を一部破壊してループを作る）
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