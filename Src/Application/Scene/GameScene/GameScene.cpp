#include "GameScene.h"
#include"../SceneManager.h"

#include "Application/Object/Character/Player/Player.h"

void GameScene::Event()
{
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

	auto player = std::make_shared<Player>();
	player->Init();
	m_objList.push_back(player);


}
