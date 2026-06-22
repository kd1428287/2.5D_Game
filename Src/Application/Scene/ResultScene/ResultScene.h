#pragma once
#include"../BaseScene/BaseScene.h"

class AutoPlayer;

class ResultScene : public BaseScene
{
public:
	ResultScene() { Init(); }
	~ResultScene() = default;

private:
	void Event(float dt) override;
	void Init()  override;

private:
	ScopedSubscriber m_endSub;
	ScopedSubscriber m_fadeInSub;
	ScopedSubscriber m_fadeOutSub;

	InScene m_state = InScene::Result;
	float m_cnt = 0.f;
	std::shared_ptr<AutoPlayer> m_player;
};