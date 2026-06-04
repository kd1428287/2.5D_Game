#pragma once

class ObjectManager;

class MapManager
{
public:
	MapManager() {};
	~MapManager() {};

	void Init();
	void GenarateMap(ObjectManager& _objManager);
};