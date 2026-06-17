#include "TitleScene.h"
#include "../SceneManager.h"

void TitleScene::Event(float dt)
{
	if (GetAsyncKeyState(VK_RETURN) & 0x8000)
	{
		SceneManager::Instance().SetNextScene
		(
			SceneManager::SceneType::Game
		);
	}
}

void TitleScene::Init()
{
}
