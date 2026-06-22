#pragma once
#include <vector>
#include <array>
#include <utility>

class ObjectManager;

// マップの生成・管理を担当するクラス
class MapManager
{
public:
	MapManager() = default;
	~MapManager() = default;

	void Init();

	std::vector<int> GenerateMap(ObjectManager& objManager);
	void GenerateResultMap(ObjectManager& objManager);
	void PlaceEventPoints(ObjectManager& objManager, int zoneDiv = 3, int pointsPerZone = 1);

private:
	// -------------------------------------------------------
	// 定数
	// -------------------------------------------------------
	static constexpr int MAP_WIDTH = 30;
	static constexpr int MAP_HEIGHT = 30;

	static constexpr float TILE_W = 0.8f;
	static constexpr float TILE_H = 0.8f;

	static constexpr int TILE_ROAD = 0;       // 道
	static constexpr int TILE_START = -1;     // 開始点（道扱い）
	static constexpr int TILE_HOME = -2;      // ホーム地点
	static constexpr int TILE_EVENT = -3;     // イベント地点
	static constexpr int TILE_BUILDING = -7;  // 孤立タイルを建物化
	static constexpr int TILE_WALL = -9;      // 外壁

	// -------------------------------------------------------
	// 共通化のための構造体・列挙型
	// -------------------------------------------------------
	struct RoadConnect
	{
		bool px, mx, pz, mz; // 接続方向: +X, -X, +Z, -Z
		int count() const { return px + mx + pz + mz; }
	};
	enum class RoadType { Straight, Curve, Junction, Cross, End, None };

	// -------------------------------------------------------
	// 内部ヘルパー
	// -------------------------------------------------------
	bool IsValidMove(int x, int y);
	bool WouldForm2x2(int x, int y);
	void GenerateRandomWalk();
	bool IsRoadTile(int t) const;
	void ClassifyTiles();
	std::vector<int> GenerateMapForMapData(ObjectManager& objManager);

	// 道パーツの形状・向き計算の共通処理
	RoadConnect GetRoadConnect(int i, int j, const int* mapData, int width, int height) const;
	std::pair<RoadType, float> GetRoadTypeAndDir(const RoadConnect& c) const;
	int GetRoadVariant(RoadType type) const;

	// -------------------------------------------------------
	// データメンバ
	// -------------------------------------------------------
	std::array<int, MAP_WIDTH* MAP_HEIGHT> mapType = {};
	std::vector<Math::Vector3>              m_eventPoints;
};