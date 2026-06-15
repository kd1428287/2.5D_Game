#include "MapManager.h"
#include "../ObjectManager.h"
#include "../../Ground/Ground.h"
#include "../../Building/Building.h"
#include "../../EventObject/DeliveryPoint.h"

void MapManager::Init() {}

void MapManager::GenarateMap(ObjectManager& _objManager)
{
	GenerateRandomWalk(2000);

	std::random_device rd;
	std::mt19937 gen(rd());

	float width = 0.8f;
	float height = 0.8f;

	// -------------------------------------------------------
	// イベント地点の配置
	// -------------------------------------------------------
	PlaceEventPoints(_objManager, 3, 1);

	// -------------------------------------------------------
	// 道パーツの接続フラグ
	// -------------------------------------------------------
	struct RoadConnect {
		bool px, mx, pz, mz; // +X, -X, +Z, -Z
		int count() const { return px + mx + pz + mz; }
	};

	auto GetRoadConnect = [&](int i, int j) -> RoadConnect {
		auto isRoad = [&](int ni, int nj) {
			if (ni < 0 || ni >= MAP_WIDTH || nj < 0 || nj >= MAP_HEIGHT) return false;
			return mapType[nj * MAP_WIDTH + ni] == 0;
			};
		return {
			isRoad(i + 1, j),  // +X
			isRoad(i - 1, j),  // -X
			isRoad(i, j + 1),  // +Z
			isRoad(i, j - 1)   // -Z
		};
		};

	// -------------------------------------------------------
	// 道パーツ種別
	// -------------------------------------------------------
	enum class RoadType { Straight, Curve, Junction, Cross, End, Not };

	auto GetRoadTypeAndDir = [](const RoadConnect& c) -> std::pair<RoadType, float> {
		int n = c.count();

		// 交差点（4方向）
		if (n == 4)
			return { RoadType::Cross, 0.0f };

		// ジャンクション（3方向）ベース定義: +X,-X,-Z (+Z欠け)
		if (n == 3) {
			if (!c.pz) return { RoadType::Junction,		0.0f };
			if (!c.mz) return { RoadType::Junction,		180.0f };
			if (!c.mx) return { RoadType::Junction,		270.0f };
			if (!c.px) return { RoadType::Junction,		90.0f };
		}

		if (n == 2) {
			// 直線（対面）
			if (c.px && c.mx) return { RoadType::Straight, 0.0f };
			if (c.pz && c.mz) return { RoadType::Straight, 90.0f };

			// カーブ（隣接）ベース定義: -X,-Z
			if (c.mx && c.mz) return { RoadType::Curve, 0.0f };
			if (c.px && c.mz) return { RoadType::Curve, 270.0f };
			if (c.px && c.pz) return { RoadType::Curve,	180.0f };
			if (c.mx && c.pz) return { RoadType::Curve, 90.0f };
		}

		if (n == 1) {
			// 行き止まり・孤立（フォールバック）
			if (c.pz) return { RoadType::End,   180.0f };
			if (c.mz) return { RoadType::End,	0.0f };
			if (c.mx) return { RoadType::End,	90.0f };
			if (c.px) return { RoadType::End,	270.0f };
		}

		return { RoadType::Not, 0.0f };
		};

	// -------------------------------------------------------
	// Buildingの向き計算用
	// -------------------------------------------------------
	struct DirInfo { int dx, dy; float dir; };
	const DirInfo directions[4] = {
		{  0, -1,   0.0f },
		{  1,  0,  90.0f },
		{  0,  1, 180.0f },
		{ -1,  0, 270.0f },
	};

	// -------------------------------------------------------
	// タイル配置
	// -------------------------------------------------------
	for (int i = 0; i < MAP_WIDTH; i++) {
		for (int j = 0; j < MAP_HEIGHT; j++) {
			Math::Vector3 pos((i - MAP_WIDTH / 2.0f) * width, 0, (j - MAP_HEIGHT / 2.0f) * height);
			int idx = j * MAP_WIDTH + i;

			// 道パーツ
			if (mapType[idx] == 0) {
				auto [roadType, roadDir] = GetRoadTypeAndDir(GetRoadConnect(i, j));
				int roadVariant = 0;
				switch (roadType) {
				case RoadType::Straight: roadVariant = 1; break;
				case RoadType::Curve:    roadVariant = 2; break;
				case RoadType::Junction: roadVariant = 3; break;
				case RoadType::Cross:    roadVariant = 4; break;
				case RoadType::End:		 roadVariant = 5; break;
				}
				if (roadType == RoadType::Not)
				{
					_objManager.CreateObject<Ground>(pos, 0, 0);
					continue;
				}
				_objManager.CreateObject<Ground>(pos, roadDir, roadVariant);
			}
			else {
				_objManager.CreateObject<Ground>(pos, 0, 0);
			}

			// 外壁
			if (i == 0 || i == MAP_WIDTH - 1 || j == 0 || j == MAP_HEIGHT - 1) {
				_objManager.CreateObject<Building>(pos + Math::Vector3(0, 0.1f, 0), 6, 0, 0.0f);
				continue;
			}

			// 建物
			if (mapType[idx] != 0) {

				// 隣接する道方向を収集
				std::vector<float> roadDirs;
				for (auto& d : directions) {
					int ni = i + d.dx;
					int nj = j + d.dy;
					if (ni >= 0 && ni < MAP_WIDTH && nj >= 0 && nj < MAP_HEIGHT) {
						if (mapType[nj * MAP_WIDTH + ni] == 0) {
							roadDirs.push_back(d.dir);
						}
					}
				}

				float dir = 0.0f;
				if (!roadDirs.empty()) {
					std::uniform_int_distribution<> dist(0, (int)roadDirs.size() - 1);
					dir = roadDirs[dist(gen)];
				}

				auto building = _objManager.CreateObject<Building>(
					pos + Math::Vector3(0, 0.1f, 0),
					(int)(KdRandom::GetFloat(3.f, 6.9f)),
					1,
					dir
				);

				// 正面方向への相対オフセット
				float rad = dir * (M_PI / 180.0f);
				Math::Vector3 frontOffset(
					-std::sin(rad) * 0.2f,
					0.0f,
					-std::cos(rad) * 0.2f
				);

				if (mapType[idx] == -2) {
					_objManager.CreateObject<DeliveryPoint>(building, frontOffset, 0.3f);
				}
			}
		}
	}

	
}

// -------------------------------------------------------
// イベント地点をゾーン分割で配置
// -------------------------------------------------------
void MapManager::PlaceEventPoints(ObjectManager& _objManager, int zoneDiv, int pointsPerZone)
{
	std::random_device rd;
	std::mt19937 gen(rd());

	int validW = MAP_WIDTH - 2;
	int validH = MAP_HEIGHT - 2;
	int zoneW = validW / zoneDiv;
	int zoneH = validH / zoneDiv;

	float width = 0.8f;
	float height = 0.8f;

	for (int zy = 0; zy < zoneDiv; ++zy) {
		for (int zx = 0; zx < zoneDiv; ++zx) {
			int xStart = 1 + zx * zoneW;
			int yStart = 1 + zy * zoneH;
			int xEnd = (zx == zoneDiv - 1) ? MAP_WIDTH - 2 : xStart + zoneW - 1;
			int yEnd = (zy == zoneDiv - 1) ? MAP_HEIGHT - 2 : yStart + zoneH - 1;

			std::vector<std::pair<int, int>> roadTiles;
			for (int y = yStart; y <= yEnd; ++y) {
				for (int x = xStart; x <= xEnd; ++x) {
					if (mapType[y * MAP_WIDTH + x] == 0) {
						roadTiles.push_back({ x, y });
					}
				}
			}

			if (roadTiles.empty()) continue;

			std::shuffle(roadTiles.begin(), roadTiles.end(), gen);
			int placeCount = std::min(pointsPerZone, (int)roadTiles.size());

			for (int k = 0; k < placeCount; ++k) {
				auto [x, y] = roadTiles[k];
				Math::Vector3 pos(
					(x - MAP_WIDTH / 2.0f) * width,
					0.1f,
					(y - MAP_HEIGHT / 2.0f) * height
				);
				mapType[y * MAP_WIDTH + x] = -2;
				m_eventPoints.push_back(pos);
			}
		}
	}
}

// -------------------------------------------------------
// 2x2の道ブロック形成チェック
// -------------------------------------------------------
bool MapManager::WouldForm2x2(int x, int y)
{
	int corners[4][2] = {
		{x - 1, y - 1}, {x, y - 1},
		{x - 1, y  }, {x, y  },
	};
	for (auto& c : corners) {
		int lx = c[0], ly = c[1];
		if (lx < 0 || lx + 1 >= MAP_WIDTH || ly < 0 || ly + 1 >= MAP_HEIGHT) continue;
		bool allRoad = true;
		for (int dy = 0; dy <= 1 && allRoad; ++dy) {
			for (int dx = 0; dx <= 1 && allRoad; ++dx) {
				int nx = lx + dx, ny = ly + dy;
				bool isCurrent = (nx == x && ny == y);
				if (!isCurrent && mapType[ny * MAP_WIDTH + nx] > 0) allRoad = false;
			}
		}
		if (allRoad) return true;
	}
	return false;
}

// -------------------------------------------------------
// 8近傍チェック
// -------------------------------------------------------
bool MapManager::IsValidMove(int x, int y)
{
	if (mapType[y * MAP_WIDTH + x] < 0) return false;
	if (WouldForm2x2(x, y)) return false;

	int count = 0;
	for (int dy = -1; dy <= 1; ++dy) {
		for (int dx = -1; dx <= 1; ++dx) {
			if (dx == 0 && dy == 0) continue;
			int nx = x + dx, ny = y + dy;
			if (nx > 0 && nx < MAP_WIDTH - 1 && ny > 0 && ny < MAP_HEIGHT - 1) {
				if (mapType[ny * MAP_WIDTH + nx] <= 0) count++;
			}
		}
	}
	return count <= 2;
}

// -------------------------------------------------------
// フロンティア展開によるマップ生成
// -------------------------------------------------------
void MapManager::GenerateRandomWalk(int steps)
{
	std::fill(mapType.begin(), mapType.end(), 1);

	mapType[15 * MAP_WIDTH + 15] = -1;
	mapType[10 * MAP_WIDTH + 10] = -2;

	std::vector<std::pair<int, int>> frontier;
	frontier.push_back({ 15, 15 });

	std::random_device rd;
	std::mt19937 gen(rd());

	int dx[] = { 1, -1, 0,  0 };
	int dy[] = { 0,  0, 1, -1 };

	while (!frontier.empty()) {
		std::uniform_int_distribution<> dist(0, (int)frontier.size() - 1);
		int idx = dist(gen);
		int cx = frontier[idx].first;
		int cy = frontier[idx].second;

		std::vector<int> validDirs;
		for (int i = 0; i < 4; ++i) {
			int nx = cx + dx[i], ny = cy + dy[i];
			if (nx > 0 && nx < MAP_WIDTH - 1 && ny > 0 && ny < MAP_HEIGHT - 1) {
				if (mapType[ny * MAP_WIDTH + nx] == 1 && IsValidMove(nx, ny)) {
					validDirs.push_back(i);
				}
			}
		}

		if (!validDirs.empty()) {
			std::uniform_int_distribution<> dirDist(0, (int)validDirs.size() - 1);
			int d = validDirs[dirDist(gen)];
			mapType[(cy + dy[d]) * MAP_WIDTH + (cx + dx[d])] = 0;
			frontier.push_back({ cx + dx[d], cy + dy[d] });
		}
		else {
			frontier[idx] = frontier.back();
			frontier.pop_back();
		}
	}

	// ループ生成（事後壁破壊）
	std::uniform_real_distribution<float> prob(0.0f, 1.0f);
	for (int y = 1; y < MAP_HEIGHT - 1; ++y) {
		for (int x = 1; x < MAP_WIDTH - 1; ++x) {
			if (mapType[y * MAP_WIDTH + x] == 1) {
				bool h = (mapType[y * MAP_WIDTH + (x - 1)] <= 0 && mapType[y * MAP_WIDTH + (x + 1)] <= 0);
				bool v = (mapType[(y - 1) * MAP_WIDTH + x] <= 0 && mapType[(y + 1) * MAP_WIDTH + x] <= 0);
				if ((h || v) && prob(gen) < 0.15f) {
					if (!WouldForm2x2(x, y)) {
						mapType[y * MAP_WIDTH + x] = 0;
					}
				}
			}
		}
	}
}