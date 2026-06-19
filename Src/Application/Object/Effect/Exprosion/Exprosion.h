#pragma once

#include "../EffectBase.h"

class Exprosion : public EffectBase
{
public:
	Exprosion() {};
	Exprosion(Math::Vector3 pos, float scale = 1.f, bool loop = false) :EffectBase(pos, scale, loop) {};
	~Exprosion()override {};

	void Init()override;
	void Update(float dt)override;
	void PostUpdate()override;

	void DrawEffect()override;
};