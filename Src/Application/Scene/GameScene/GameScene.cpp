#include "GameScene.h"
#include"../SceneManager.h"

#include "Application/Object/Character/Player/Player.h"
#include "Application/Object/Ground/GroundBase.h"

#include "Application/System/InputManager/InputManager.h"
#include "Application/System/CameraManager/CameraManager.h"

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
	m_camera = std::make_unique<KdCamera>();
	m_camera->SetProjectionMatrix(60);

	CameraManager::Instance().Init();

	auto player = std::make_shared<Player>();
	player->Init();
	m_objList.push_back(player);

	auto ground = std::make_shared<GroundBase>();
	ground->Init();
	m_objList.push_back(ground);


}
