#include "DeliveryScoreUI.h"
#include "../../../../Reader/Reader.h"

void DeliveryScoreUI::RebuildMatrix()
{
	Math::Matrix mat =
		Math::Matrix::CreateScale(m_animScale) *
		Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_dir + m_animExtraDir)) *
		Math::Matrix::CreateTranslation(m_pos + m_animOffset);

	m_mWorld = mat;
}

void DeliveryScoreUI::Init()
{
	m_pos = { 1.5f,0.5f,1.5f };

	// 十の位と一の位をそれぞれ初期化
	m_numberTens = std::make_shared<Number>(m_pos, m_score / 10, m_dir);
	m_numberTens->Init();

	m_numberOnes = std::make_shared<Number>(m_pos, m_score % 10, m_dir);
	m_numberOnes->Init();

	m_resPrbSub =
		GLOBALEVENT.subscribe<Events::Else::ResultPlayerProduction>([this](const Events::Else::ResultPlayerProduction& e)
			{
				switch (e.m_state)
				{
				case Events::Else::ResultPlayerProduction::State::Delivery:
					m_pendingScore = Reader::Instance().ReadScoreForPrd();
					m_dropState = DropState::DroppingDown;
					m_dropTimer = 0.f;
					break;

				case Events::Else::ResultPlayerProduction::State::Completed:
					m_spinState = SpinState::Spinning;
					m_spinTimer = 0.f;
					break;

				default:
					break;
				}
			});
}

void DeliveryScoreUI::Update(float dt)
{
	// ---- 演出1 : ドロップ ----
	switch (m_dropState)
	{
	case DropState::DroppingDown:
	{
		m_dropTimer += dt;
		float t = std::min(m_dropTimer / DROP_DURATION, 1.f);
		m_animOffset.y = -DROP_AMOUNT * (t * t);

		if (t >= 1.f)
		{
			// スコアを桁ごとに分解して適用
			m_numberTens->SetNumber(m_pendingScore / 10);
			m_numberOnes->SetNumber(m_pendingScore % 10);

			m_score = m_pendingScore;
			m_dropState = DropState::WaitBottom;
			m_dropTimer = 0.f;
		}
		break;
	}
	case DropState::WaitBottom:
	{
		m_dropTimer += dt;
		if (m_dropTimer >= DROP_HOLD)
		{
			m_dropState = DropState::RisingUp;
			m_dropTimer = 0.f;
		}
		break;
	}
	case DropState::RisingUp:
	{
		m_dropTimer += dt;
		float t = std::min(m_dropTimer / RISE_DURATION, 1.f);
		float inv = 1.f - t;
		m_animOffset.y = -DROP_AMOUNT * (inv * inv);

		if (t >= 1.f)
		{
			m_animOffset.y = 0.f;
			m_dropState = DropState::Idle;
		}
		break;
	}
	default:
		break;
	}

	// ---- 演出2 : スピン ----
	if (m_spinState == SpinState::Spinning)
	{
		m_spinTimer += dt;
		float t = std::min(m_spinTimer / SPIN_DURATION, 1.f);

		m_animExtraDir = 360.f * t;
		m_animScale = 1.f + (SPIN_PEAK_SCALE - 1.f) * std::sin(DirectX::XM_PI * t);

		if (t >= 1.f)
		{
			m_animExtraDir = 0.f;
			m_animScale = 1.f;
			m_spinState = SpinState::Idle;
			GLOBALEVENT.publish(Events::Else::ResultPlayerProduction(Events::Else::ResultPlayerProduction::State::Add));
		}
	}

	// 基準となるワールド行列を計算
	RebuildMatrix();

	// 行列の更新
	if (m_score >= 10)
	{
		// 2桁のときは、中心(m_mWorld)から左右に半分ずつずらす
		float halfGapX = GAP_WIDTH * 0.05f;
		float halfGap = GAP_WIDTH * 0.5f;
		m_numberTens->SetMatrix(Math::Matrix::CreateTranslation(-halfGapX, 0, halfGap) * m_mWorld);
		m_numberOnes->SetMatrix(Math::Matrix::CreateTranslation(halfGapX, 0, -halfGap) * m_mWorld);
	}
	else
	{
		// 1桁のときは、一の位をジャストセンターに配置
		m_numberOnes->SetMatrix(m_mWorld);
	}
}

void DeliveryScoreUI::GenerateDepthMapFromLight()
{
	// 10以上のときのみ十の位を描画・計算する
	if (m_score >= 10) m_numberTens->GenerateDepthMapFromLight();
	m_numberOnes->GenerateDepthMapFromLight();
}

void DeliveryScoreUI::DrawLit()
{
	// 10以上のときのみ十の位を描画する
	if (m_score >= 10) m_numberTens->DrawLit();
	m_numberOnes->DrawLit();
}