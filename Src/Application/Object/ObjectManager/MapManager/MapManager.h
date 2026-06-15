#pragma once

class ObjectManager;

class MapManager
{
public:
	MapManager() {};
	~MapManager() {};

	void Init();
	void GenarateMap(ObjectManager& _objManager);
	void PlaceEventPoints(ObjectManager& _objManager, int zoneDiv = 3, int pointsPerZone = 1);

private:
	bool IsValidMove(int x, int y);
	bool WouldForm2x2(int x, int y);
	void GenerateRandomWalk(int steps);
	void SetUpPresetPoint();

	static const int MAP_WIDTH = 30;
	static const int MAP_HEIGHT = 30;
	std::array<int, MAP_WIDTH* MAP_HEIGHT> mapType = {};
	std::vector<Math::Vector3> m_presetPoint;
	std::vector<Math::Vector3> m_eventPoints;
};