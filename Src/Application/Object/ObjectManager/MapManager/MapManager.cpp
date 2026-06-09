#include "MapManager.h"
#include "../ObjectManager.h"
#include "../../Ground/Ground.h"
#include "../../Building/Building.h"

void MapManager::Init()
{

}

void MapManager::GenarateMap(ObjectManager& _objManager)
{
	int MAP_WIDTH = 30;
	int MAP_HEIGHT = 30;
	float width = 0.8f;
	float height = 0.8f;
	Math::Vector3 pos;
	pos.y = 0;

	pos.x = MAP_WIDTH / 2.0f * width;
	pos.z = MAP_HEIGHT / 2.0f * height;

	for (int i = 0; i < MAP_WIDTH; i++)
	{
		pos.x = (i - MAP_WIDTH / 2.0f) * width;
		for (int j = 0; j < MAP_HEIGHT; j++)
		{
			pos.z = (j - MAP_HEIGHT / 2.0f) * height;
			
			_objManager.CreateObject<Ground>(pos, 0);
			if (i == 0 || i == MAP_WIDTH - 1 || j == 0 || j == MAP_HEIGHT - 1)
			{
				_objManager.CreateObject<Building>(pos + Math::Vector3(0, 0.1f, 0),6);
				continue;
			}
			if (j % 2)continue;
			_objManager.CreateObject<Building>(pos + Math::Vector3(0,0.1f,0));
		
		}
	}
}