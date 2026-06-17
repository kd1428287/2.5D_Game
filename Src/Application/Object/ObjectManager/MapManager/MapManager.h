#pragma once

class ObjectManager;

// マップの生成・管理を担当するクラス
class MapManager
{
public:
	MapManager() = default;
	~MapManager() = default;

	void Init();
	std::vector<int> GenarateMap(ObjectManager& objManager);
	void PlaceEventPoints(ObjectManager& objManager, int zoneDiv = 3, int pointsPerZone = 1);

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
	static constexpr int TILE_ROAD = 0;   // 道
	static constexpr int TILE_START = -1;   // 開始点（道扱い）
	static constexpr int TILE_HOME = -2;   // 開始点
	static constexpr int TILE_EVENT = -3;   // イベント地点
	static constexpr int TILE_BUILDING = -7;   // 道/空き地を建物化（孤立タイル）
	static constexpr int TILE_WALL = -9;   // 外壁
	// 1 以上: 建物

	// -------------------------------------------------------
	// 内部ヘルパー
	// -------------------------------------------------------
	bool IsValidMove(int x, int y);
	bool WouldForm2x2(int x, int y);
	void GenerateRandomWalk();

	// タイプ分け確定フェーズ
	// ・四方すべてが道（ROAD/START/EVENT）の道タイル → TILE_BUILDING
	// ・四方すべてが非正数（道/空き地系）の空き地（tile <= 0 かつ道でない）→ TILE_BUILDING
	void ClassifyTiles();

	// -------------------------------------------------------
	// データメンバ
	// -------------------------------------------------------
	std::array<int, MAP_WIDTH* MAP_HEIGHT> mapType = {};
	std::vector<Math::Vector3>              m_eventPoints;
};