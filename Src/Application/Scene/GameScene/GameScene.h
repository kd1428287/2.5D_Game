#pragma once

#include"../BaseScene/BaseScene.h"
#include"../../System/TimeManager/TimeManager.h"

class GameScene : public BaseScene
{
public:
	GameScene() { Init(); }
	~GameScene() = default;

private:
	void Event(float dt) override;
	void Init()  override;

private:
	std::unique_ptr<SpawnManager> m_spawnManager = nullptr;
	std::unique_ptr<TimeManager>  m_timeManager = nullptr;

	InScene m_state = InScene::Title;

	float m_resultTransitionTimer = 0.f;    // 変数名を意図が分かる名前に変更

	ScopedSubscriber m_endSub;
	ScopedSubscriber m_fadeInSub;
	ScopedSubscriber m_fadeOutSub;
};