#include "Meter.h"

void MeterUI::Init()
{
	m_pos = { -300,-300 };
	m_texture = KdAssets::Instance().m_textures.GetData("Asset/Textures/Game/meter.png");
	m_needle = KdAssets::Instance().m_textures.GetData("Asset/Textures/Game/meter.png");
	m_speedSub = GLOBALEVENT.subscribe<Events::Player::ChangeSpeedLevel>([this](const Events::Player::ChangeSpeedLevel& e)
		{
			SetTargetPoint((NeedlePoint)e.level);
		});
}

void MeterUI::Update(float dt)
{

}

void MeterUI::DrawSprite()
{
	/*Math::Matrix mat =
		Math::Matrix::CreateScale(m_scale) *
		Math::Matrix::CreateTranslation(m_pos);*/
	KdShaderManager::Instance().m_spriteShader.DrawTex(m_texture, m_pos.x,m_pos.y);
}

void MeterUI::SetTargetPoint(NeedlePoint point)
{
	switch (point)
	{
	case MeterUI::NeedlePoint::Idle:
		break;
	case MeterUI::NeedlePoint::Speed1:
		break;
	case MeterUI::NeedlePoint::Speed2:
		break;
	case MeterUI::NeedlePoint::Speed3:
		break;
	case MeterUI::NeedlePoint::Speed4:
		break;
	case MeterUI::NeedlePoint::Speed5:
		break;
	case MeterUI::NeedlePoint::Speed6:
		break;
	case MeterUI::NeedlePoint::Clash:
		break;
	default:
		break;
	}
}
