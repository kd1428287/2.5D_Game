#pragma once

class ObjectManager;

// マップの生成・管理を担当するクラス
class MapManager
{
public:
	MapManager() = default;
	~MapManager() = default;

	void Init();

	// スペルミス修正: Genarate → Generate
	std::vector<int> GenerateMap(ObjectManager& objManager);

	// PlaceEventPoints は GenerateMap 内から呼ぶが、
	// 外部から呼び出せるよう public に残す（引数デフォルト値付き）
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
	static constexpr int TILE_ROAD = 0;  // 道
	static constexpr int TILE_START = -1;  // 開始点（道扱い）
	static constexpr int TILE_HOME = -2;  // ホーム地点
	static constexpr int TILE_EVENT = -3;  // イベント地点（建物タイルに付与）
	static constexpr int TILE_BUILDING = -7;  // 孤立タイルを建物化
	static constexpr int TILE_WALL = -9;  // 外壁
	// 1 以上: 建物（未確定の空き地）

	// -------------------------------------------------------
	// 内部ヘルパー
	// -------------------------------------------------------
	bool IsValidMove(int x, int y);
	bool WouldForm2x2(int x, int y);
	void GenerateRandomWalk();

	// 道タイル判定（ROAD / START / EVENT は道扱い）
	bool IsRoadTile(int t) const;

	// タイプ分け確定フェーズ:
	//   空き地タイル（>= 1）で四方すべてが非正数 → TILE_BUILDING
	void ClassifyTiles();

	// -------------------------------------------------------
	// データメンバ
	// -------------------------------------------------------
	std::array<int, MAP_WIDTH* MAP_HEIGHT> mapType = {};
	std::vector<Math::Vector3>              m_eventPoints;
};