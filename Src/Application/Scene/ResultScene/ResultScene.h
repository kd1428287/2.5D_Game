#pragma once
#include"../BaseScene/BaseScene.h"

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

	InScene m_state = InScene::Result;
	float m_cnt = 0.f;
};