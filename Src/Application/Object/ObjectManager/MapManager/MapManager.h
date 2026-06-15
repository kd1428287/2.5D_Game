#pragma once

class ObjectManager;

class MapManager
{
public:
	MapManager() {};
	~MapManager() {};

	void Init();
	void GenarateMap(ObjectManager& _objManager);

private:
	// 近傍チェック関数（8近傍）
	bool IsValidMove(int x, int y);
	void GenerateRandomWalk(int steps);

	static const int MAP_WIDTH = 30;
	static const int MAP_HEIGHT = 30;
	std::array<int, MAP_WIDTH* MAP_HEIGHT> mapType = {};
};