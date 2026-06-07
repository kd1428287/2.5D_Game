#include "MapManager.h"
#include "../ObjectManager.h"
#include "../../Ground/Ground.h"
#include "../../Building/Building.h"

void MapManager::Init()
{

}

void MapManager::GenarateMap(ObjectManager& _objManager)
{
	float mapWidth = 8;
	Math::Vector3 pos;
	pos.y = -5;

	for (int i = 0; i < 10; i++)
	{
		pos.x = i * mapWidth;
		for (int j = 0; j < 10; j++)
		{
			pos.z = j * mapWidth;
			
			_objManager.CreateObject<Ground>(pos, 0);
			_objManager.CreateObject<Building>(pos + Math::Vector3(0,1,0));
		
		}
	}
}