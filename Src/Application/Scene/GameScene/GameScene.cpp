#include "GameScene.h"
#include"../SceneManager.h"

#include "Application/Object/Character/Player/Player.h"
#include "Application/Object/Ground/Ground.h"
#include "Application/Object/SkySphere/SkySphere.h"

#include "Application/System/InputManager/InputManager.h"
#include "Application/System/CameraManager/CameraManager.h"
#include "Application/Object/ObjectManager/ObjectManager.h"
#include "Application/Object/ObjectManager/MapManager/MapManager.h"


void GameScene::Event()
{
	InputManager::Instance().Update();

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
	
	Math::Vector3 pos = { 0,2,0 };

	auto player = m_objectManager->CreateObject<Player>(pos);

	m_cameraManager->SetCameraTarget(player);

	m_mapManager->GenarateMap(*m_objectManager);

	auto sky = m_objectManager->CreateObject<SkySphere>();

	
}
