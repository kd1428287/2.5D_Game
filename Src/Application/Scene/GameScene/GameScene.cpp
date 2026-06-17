#include "GameScene.h"
#include"../SceneManager.h"

#include "Application/Object/Character/Player/Player.h"
#include "Application/Object/Ground/Ground.h"
#include "Application/Object/SkySphere/SkySphere.h"

#include "Application/System/InputManager/InputManager.h"
#include "Application/System/TimeManager/TimeManager.h"
#include "Application/System/CameraManager/CameraManager.h"
#include "Application/Object/ObjectManager/ObjectManager.h"
#include "Application/Object/ObjectManager/MapManager/MapManager.h"


void GameScene::Event(float dt)
{
	InputManager::Instance().Update();
	m_spawnManager->Update(dt);
	m_timeManager->Update(dt);

	if (GetAsyncKeyState('T') & 0x8000)
	{
		SceneManager::Instance().SetNextScene
		(
			SceneManager::SceneType::Title
		);
	}
}

void GameScene::Init()
{
	BaseScene::Init();

	m_spawnManager = std::make_unique<SpawnManager>(m_objectManager);
	m_spawnManager->Init();
	m_timeManager = std::make_unique<TimeManager>();
	m_timeManager->Init();

	Math::Vector3 pos = { 0,1,0 };

	auto player = m_objectManager->CreateObject<Player>(pos);

	m_cameraManager->SetCameraTarget(player);

	auto map = m_mapManager->GenarateMap(*m_objectManager);
	m_spawnManager->SetMapData(map);

	auto sky = m_objectManager->CreateObject<SkySphere>();
}
