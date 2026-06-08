#include "MapManager.h"
#include "../ObjectManager.h"
#include "../../Ground/Ground.h"
#include "../../Building/Building.h"

void MapManager::Init()
{

}

void MapManager::GenarateMap(ObjectManager& _objManager)
{
	int MAP_WIDTH = 1;
	int MAP_HEIGHT = 1;
	float width = 0.8f;
	float height = 0.8f;
	Math::Vector3 pos;
	pos.y = -5;

	pos.x = MAP_WIDTH / 2.0f * width;
	pos.z = MAP_HEIGHT / 2.0f * height;

	for (int i = 0; i < MAP_WIDTH; i++)
	{
		pos.x = (i - MAP_WIDTH / 2.0f) * width;
		for (int j = 0; j < MAP_HEIGHT; j++)
		{
			pos.z = (j - MAP_HEIGHT / 2.0f) * height;
			
			_objManager.CreateObject<Ground>(pos, 0);
			if (j % 2)continue;
			_objManager.CreateObject<Building>(pos + Math::Vector3(0,1,0));
		
		}
	}
}