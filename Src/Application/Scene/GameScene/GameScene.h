#pragma once

#include"../BaseScene/BaseScene.h"
#include"../../System/TimeManager/TimeManager.h"

class GameScene : public BaseScene
{
public :

	GameScene()  { Init(); }
	~GameScene() {}

private:

	void Event(float dt) override;
	void Init()  override;

private:
	std::unique_ptr<SpawnManager>m_spawnManager = nullptr;
	std::unique_ptr<TimeManager>m_timeManager = nullptr;
};
