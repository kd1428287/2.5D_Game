#pragma once

#include "../../UIObject.h"

class MeterUI : public UIObject
{
public:
	MeterUI() {};
	~MeterUI()override {};

	void Init()override;
	void Update(float dt)override;
	void DrawSprite()override;

private:
	enum class NeedlePoint
	{
		Idle,
		Speed1,
		Speed2,
		Speed3,
		Speed4,
		Speed5,
		Speed6,
		Clash,
	};

	void SetTargetPoint(NeedlePoint point);

	std::shared_ptr<KdTexture> m_needle;
	Math::Vector2 m_needlePos;
	Math::Vector2 m_needleTargetPos;
	Math::Vector2 m_needleAngle;
	Math::Vector2 m_needleTargetAngle;

	ScopedSubscriber m_speedSub;
};