#include "MapManager.h"
#include "../ObjectManager.h"
#include "../../Ground/Ground.h"

void MapManager::Init()
{

}

void MapManager::GenarateMap(ObjectManager& _objManager)
{
	float mapWidth = 8;
	Math::Vector3 pos;
	pos.y = -5;

	for (int i = 0; i < 50; i++)
	{
		pos.x = i * mapWidth;
		for (int j = 0; j < 50; j++)
		{
			pos.z = j * mapWidth;
			
			_objManager.CreateObject<Ground>(pos, 1);
		
		}
	}
}