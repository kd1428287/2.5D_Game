#include "MeterUI.h"

// --- arrow の目標座標テーブル ---
static const Math::Vector2 s_arrowPoints[] =
{
	{  54.0f, -35.0f },  // Idle
	{  36.0f, -30.0f },  // Speed1
	{  18.0f, -25.0f },  // Speed2
	{  0.0f , -10.0f },  // Speed3
	{ -19.0f,  10.0f },  // Speed4
	{ -37.0f,  35.0f },  // Speed5
	{ -55.0f,  60.0f },  // Speed6
	{  54.0f, -35.0f },  // Clash
};

void MeterUI::Init()
{
	// --- メーター本体: 画面左下 ---
	m_pos = { -420.0f, -200.0f, 0.0f };

	m_texture = KdAssets::Instance().m_textures.GetData("Asset/Textures/UI/Game/speedMeter.png");
	m_needle = KdAssets::Instance().m_textures.GetData("Asset/Textures//UI/Game/arrow.png");

	// arrow の初期位置は Idle に合わせる
	m_arrowPos = s_arrowPoints[static_cast<int>(NeedlePoint::Idle)];
	m_arrowTargetPos = m_arrowPos;

	// SpeedLevel 変化イベントを購読
	m_speedSub = GLOBALEVENT.subscribe<Events::Player::ChangeSpeedLevel>(
		[this](const Events::Player::ChangeSpeedLevel& e)
		{
			SetTargetPoint(static_cast<NeedlePoint>(e.level));
		});
}

void MeterUI::Update(float dt)
{
	// arrow を目標位置へ線形補間 (Lerp) で滑らかに移動
	m_arrowPos.x += (m_arrowTargetPos.x - m_arrowPos.x) * m_arrowLerpSpeed * dt;
	m_arrowPos.y += (m_arrowTargetPos.y - m_arrowPos.y) * m_arrowLerpSpeed * dt;
}

void MeterUI::DrawSprite()
{
	Math::Matrix mat =
		Math::Matrix::CreateScale(3.f) *
		Math::Matrix::CreateTranslation(m_pos);

	KdShaderManager::Instance().m_spriteShader.SetMatrix(mat);
	KdShaderManager::Instance().m_spriteShader.DrawTex(m_texture,0,0);

	KdShaderManager::Instance().m_spriteShader.DrawTex(
		m_needle, m_arrowPos.x, m_arrowPos.y);

	KdShaderManager::Instance().m_spriteShader.SetMatrix(Math::Matrix::Identity);
	return;
}

void MeterUI::SetTargetPoint(NeedlePoint point)
{
	// テーブルから目標座標を取得してセット
	int idx = static_cast<int>(point);
	m_arrowTargetPos = s_arrowPoints[idx];
}