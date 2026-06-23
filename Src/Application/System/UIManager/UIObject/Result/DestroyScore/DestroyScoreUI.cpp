#include "DestroyScoreUI.h"
#include "../../../../Reader/Reader.h"

// ----------------------------------------------------------------
// m_center と現在の m_digitCount から桁ごとのX座標を返す
//
// 全桁を中心揃えするために、全体幅の半分だけオフセットする。
//   全体幅 = DIGIT_SPACING * (m_digitCount - 1)
//   右端(一の位) のX = m_center.x + 全体幅 / 2
//   各桁のX         = 右端X - DIGIT_SPACING * digitIndex
// ----------------------------------------------------------------
float DestroyScoreUI::CalcDigitX(int digitIndex) const
{
	float totalWidth = DIGIT_SPACING * (m_digitCount - 1);
	float rightEdgeX = m_center.x + totalWidth * 0.5f;
	return rightEdgeX - DIGIT_SPACING * digitIndex;
}

// ----------------------------------------------------------------
void DestroyScoreUI::SetScore(int score)
{
	m_score = std::clamp(score, 0, 9999);

	// 桁数を計算（最低1桁）
	m_digitCount = 1;
	for (int v = m_score; v >= 10; v /= 10) { ++m_digitCount; }
	m_digitCount = std::min(m_digitCount, MAX_DIGITS);

	// 各桁の数値を取り出して Number を生成
	int v = m_score;
	for (int i = 0; i < MAX_DIGITS; ++i)
	{
		int digit = v % 10;
		v /= 10;

		// 桁数が確定した後に CalcDigitX で中心基準のX座標を求める
		Math::Vector3 pos = m_center;
		pos.x = CalcDigitX(i);

		m_digits[i] = std::make_shared<Number>(pos, digit, m_dir);
		m_digits[i]->Init();
	}
}

// ----------------------------------------------------------------
void DestroyScoreUI::UpdateMatrix()
{
	for (int i = 0; i < m_digitCount; ++i)
	{
		auto& n = m_digits[i];
		if (!n) { continue; }

		n->m_mWorld =
			Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_dir)) *
			Math::Matrix::CreateTranslation(n->m_pos);
	}
}

// ----------------------------------------------------------------
void DestroyScoreUI::Init()
{
	m_center = { 0,3.0f,0 };
	int rawScore = static_cast<int>(Reader::Instance().ReadScore().y);
	SetScore(rawScore);

	m_resPrbSub =
		GLOBALEVENT.subscribe<Events::Else::ResultPlayerProduction>([this](const Events::Else::ResultPlayerProduction& e)
			{
				if (e.m_state != Events::Else::ResultPlayerProduction::State::Add)return;
				if (!m_isFalling)
				{
					m_isFalling = true;
					m_velocity = { 0.f, INITIAL_VEL_Y, 0.f };
				}
			});
}

// ----------------------------------------------------------------
void DestroyScoreUI::Update(float dt)
{
	if (!m_isFalling) { return; }

	m_velocity.y += GRAVITY * dt;

	for (int i = 0; i < m_digitCount; ++i)
	{
		auto& n = m_digits[i];
		if (!n) { continue; }

		n->m_pos += m_velocity * dt;
	}

	UpdateMatrix();
}

// ----------------------------------------------------------------
void DestroyScoreUI::GenerateDepthMapFromLight()
{
	for (int i = 0; i < m_digitCount; ++i)
	{
		if (m_digits[i]) { m_digits[i]->GenerateDepthMapFromLight(); }
	}
}

void DestroyScoreUI::DrawLit()
{
	for (int i = 0; i < m_digitCount; ++i)
	{
		if (m_digits[i]) { m_digits[i]->DrawLit(); }
	}
}