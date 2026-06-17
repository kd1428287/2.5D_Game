#pragma once

#include"../BaseScene/BaseScene.h"

class TitleScene : public BaseScene
{
public :

	TitleScene()  { Init(); }
	~TitleScene() {}

private :

	void Event(float dt) override;
	void Init()  override;
};
