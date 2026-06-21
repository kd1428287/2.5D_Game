#include "DeliveryScoreUI.h"
#include "../../../../Reader/Reader.h"
#include <cstdio>
#include <string>
#include <cstdlib>
#include <algorithm>

// ----------------------------------------------------------------
// ヘルパー
// ----------------------------------------------------------------
void DeliveryScoreUI::SetupDigits(int value)
{
	m_digitCount = 0;
	int calc = value;
	do { m_digitCount++; calc /= 10; } while (calc > 0);

	int tmp = value;
	for (int i = m_digitCount - 1; i >= 0; --i)
	{
		m_digits[i] = tmp % 10;
		tmp /= 10;
	}
}

// ----------------------------------------------------------------
// Init
// ----------------------------------------------------------------
void DeliveryScoreUI::Init()
{
	m_score = static_cast<int>(Reader::Instance().ReadScore().x);
	SetupDigits(m_score);

	m_fixedFromRight = 0;
	m_digitTimer = 0.f;
	m_rollTimer = 0.f;
	m_phaseWait = 0.f;
	m_bonusTimer = 0.f;
	m_bonusScale = 0.f;
	m_bonusAlpha = 0.f;
	for (int i = 0; i < MAX_DIGITS; ++i)
		m_currentRand[i] = 0;

	m_phase = RollPhase::Waiting;
	m_pos = { 0.f, 0.f };

	m_rollSub = GLOBALEVENT.subscribe<Events::Else::DeliveryScoreRollBegin>(
		[this](const Events::Else::DeliveryScoreRollBegin&)
		{
			m_phase = RollPhase::FirstRoll;
			for (int i = 0; i < MAX_DIGITS; ++i)
				m_currentRand[i] = rand() % 10;
		}
	);
}

// ----------------------------------------------------------------
// Update
// ----------------------------------------------------------------
void DeliveryScoreUI::Update(float dt)
{
	switch (m_phase)
	{
	case RollPhase::Waiting:
		break;

		// --------------------------------------------------
	case RollPhase::FirstRoll:
	{
		m_rollTimer += dt;
		if (m_rollTimer >= m_rollInterval)
		{
			m_rollTimer -= m_rollInterval;
			for (int i = 0; i < MAX_DIGITS; ++i)
				if ((MAX_DIGITS - 1 - i) >= m_fixedFromRight)
					m_currentRand[i] = rand() % 10;
		}

		m_digitTimer += dt;
		if (m_digitTimer >= m_digitDelay)
		{
			m_digitTimer -= m_digitDelay;
			m_fixedFromRight++;
			if (m_fixedFromRight >= MAX_DIGITS)
			{
				m_fixedFromRight = MAX_DIGITS;
				m_phaseWait = 0.f;
				m_phase = RollPhase::WaitBetween;
			}
		}
		break;
	}

	// --------------------------------------------------
	case RollPhase::WaitBetween:
	{
		m_phaseWait += dt;
		if (m_phaseWait >= m_phaseWaitMax)
		{
			m_bonusTimer = 0.f;
			m_bonusScale = 0.f;
			m_bonusAlpha = 0.f;
			m_phase = RollPhase::ShowTimeBonus;
		}
		break;
	}

	// --------------------------------------------------
	case RollPhase::ShowTimeBonus:
	{
		m_bonusTimer += dt;

		const float scaleUpEnd = 0.4f;
		const float holdEnd = 1.4f;
		const float fadeEnd = m_bonusDuration; // 1.8f

		if (m_bonusTimer < scaleUpEnd)
		{
			// スケールアップ： 0→1
			float t = m_bonusTimer / scaleUpEnd;
			m_bonusScale = t;
			m_bonusAlpha = t;
		}
		else if (m_bonusTimer < holdEnd)
		{
			// 静止
			m_bonusScale = 1.f;
			m_bonusAlpha = 1.f;
		}
		else if (m_bonusTimer < fadeEnd)
		{
			// フェードアウト
			float t = 1.f - (m_bonusTimer - holdEnd) / (fadeEnd - holdEnd);
			m_bonusScale = 1.f + (1.f - t) * 0.3f; // 少し膨張させながら消す
			m_bonusAlpha = t;
		}
		else
		{
			// 演出終了 → time取得・最終スコア計算
			m_bonusAlpha = 0.f;
			float time = 1.f + (180.f - Reader::Instance().ReadTime()) * 0.1f;
			m_finalScore = static_cast<int>(m_score * time);
			SetupDigits(m_finalScore);

			m_phaseWait = 0.f;
			m_phase = RollPhase::WaitBetween2;
		}
		break;
	}

	// --------------------------------------------------
	case RollPhase::WaitBetween2:
	{
		m_phaseWait += dt;
		if (m_phaseWait >= m_phaseWaitMax)
		{
			m_fixedFromRight = 0;
			m_digitTimer = 0.f;
			m_rollTimer = 0.f;
			for (int i = 0; i < MAX_DIGITS; ++i)
				m_currentRand[i] = rand() % 10;
			m_phase = RollPhase::SecondRoll;
		}
		break;
	}

	// --------------------------------------------------
	case RollPhase::SecondRoll:
	{
		m_rollTimer += dt;
		if (m_rollTimer >= m_rollInterval)
		{
			m_rollTimer -= m_rollInterval;
			for (int i = 0; i < MAX_DIGITS; ++i)
				if ((MAX_DIGITS - 1 - i) >= m_fixedFromRight)
					m_currentRand[i] = rand() % 10;
		}

		m_digitTimer += dt;
		if (m_digitTimer >= m_digitDelay)
		{
			m_digitTimer -= m_digitDelay;
			m_fixedFromRight++;
			if (m_fixedFromRight >= MAX_DIGITS)
			{
				m_fixedFromRight = MAX_DIGITS;
				m_phase = RollPhase::Done;
				GLOBALEVENT.publish(Events::Else::DeliveryScoreRollEnd());
				GLOBALEVENT.publish(Events::Else::DestroyScoreRollBegin());
			}
		}
		break;
	}

	case RollPhase::Done:
		break;
	}
}

// ----------------------------------------------------------------
// DrawSprite
// ----------------------------------------------------------------
void DeliveryScoreUI::DrawSprite()
{
	// --- スコア行 ---
	{
		char scoreBuf[MAX_DIGITS + 1] = {};
		for (int i = 0; i < MAX_DIGITS; ++i)
		{
			int scoreIndex = i - (MAX_DIGITS - m_digitCount);
			int fromRight = MAX_DIGITS - 1 - i;

			if (fromRight < m_fixedFromRight)
				scoreBuf[i] = (scoreIndex < 0) ? '0' : ('0' + m_digits[scoreIndex]);
			else
				scoreBuf[i] = '0' + m_currentRand[i];
		}
		scoreBuf[MAX_DIGITS] = '\0';

		std::string s = "DeliveryScore :     ";
		s += scoreBuf;

		int outlineSize = 7;
		auto sprite = KdFontManager::Instance().CreateFontTexture(5, s, 1, outlineSize);

		int totalWidth = 0;
		for (auto& charData : sprite->GetTexList())
			if (charData->FontTex)
				totalWidth += charData->FontTex->GetInfo().Width;

		int offsetX = -totalWidth / 2;
		for (auto& charData : sprite->GetTexList())
		{
			if (!charData->FontTex) continue;
			if (charData->OutlineTex)
				KdShaderManager::Instance().m_spriteShader.DrawTex(
					charData->OutlineTex, m_pos.x + offsetX, m_pos.y);
			KdShaderManager::Instance().m_spriteShader.DrawTex(
				charData->FontTex, m_pos.x + offsetX, m_pos.y);
			offsetX += charData->FontTex->GetInfo().Width;
		}
	}

	// --- TimeBonus 演出 ---
	if (m_phase == RollPhase::ShowTimeBonus && m_bonusAlpha > 0.f)
	{
		std::string bonusStr = "Time Bonus!!";
		int outlineSize = 10;
		auto sprite = KdFontManager::Instance().CreateFontTexture(5, bonusStr, 1, outlineSize);

		int totalWidth = 0;
		for (auto& charData : sprite->GetTexList())
			if (charData->FontTex)
				totalWidth += charData->FontTex->GetInfo().Width;

		// スケール行列を適用
		Math::Matrix scaleMat = Math::Matrix::CreateScale(m_bonusScale, m_bonusScale, 1.f);
		KdShaderManager::Instance().m_spriteShader.SetMatrix(scaleMat);

		Math::Color color = { 1.f, 0.9f, 0.2f, m_bonusAlpha }; // 黄色
		int offsetX = -static_cast<int>(totalWidth * m_bonusScale) / 2;

		for (auto& charData : sprite->GetTexList())
		{
			if (!charData->FontTex) continue;
			if (charData->OutlineTex)
				KdShaderManager::Instance().m_spriteShader.DrawTex(
					charData->OutlineTex,
					m_pos.x + offsetX,
					m_pos.y + 80.f, // スコア行の少し上
					nullptr,
					&color);
			KdShaderManager::Instance().m_spriteShader.DrawTex(
				charData->FontTex,
				m_pos.x + offsetX,
				m_pos.y + 80.f,
				nullptr,
				&color);
			offsetX += charData->FontTex->GetInfo().Width;
		}

		// 行列をリセット
		KdShaderManager::Instance().m_spriteShader.SetMatrix(Math::Matrix::Identity);
	}
}