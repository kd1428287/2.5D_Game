#include "DestroyScoreUI.h"
#include "../../../../Reader/Reader.h"
#include <cstdio>
#include <string>
#include <cstdlib>

void DestroyScoreUI::Init()
{
	m_score = static_cast<int>(Reader::Instance().ReadScore().y);

	m_digitCount = 0;
	int calc = m_score;
	do { m_digitCount++; calc /= 10; } while (calc > 0);

	int tmp = m_score;
	for (int i = m_digitCount - 1; i >= 0; --i)
	{
		m_digits[i] = tmp % 10;
		tmp /= 10;
	}

	m_fixedFromRight = 0;
	m_digitTimer = 0.f;
	m_rollTimer = 0.f;
	for (int i = 0; i < MAX_DIGITS; ++i)
		m_currentRand[i] = 0; // ロール前は全桁0

	m_rolling = false;
	m_finished = false;

	m_pos = { 0.f, -100.f };

	m_rollSub = GLOBALEVENT.subscribe<Events::Else::DestroyScoreRollBegin>(
		[this](const Events::Else::DestroyScoreRollBegin&)
		{
			m_rolling = true;
		}
	);
}

void DestroyScoreUI::Update(float dt)
{
	if (!m_rolling || m_finished) return;

	m_rollTimer += dt;
	if (m_rollTimer >= m_rollInterval)
	{
		m_rollTimer -= m_rollInterval;
		for (int i = 0; i < MAX_DIGITS; ++i)
		{
			int fromRight = MAX_DIGITS - 1 - i;
			if (fromRight >= m_fixedFromRight)
				m_currentRand[i] = rand() % 10; // ロール中はランダム
		}
	}

	m_digitTimer += dt;
	if (m_digitTimer >= m_digitDelay)
	{
		m_digitTimer -= m_digitDelay;
		m_fixedFromRight++;

		if (m_fixedFromRight >= MAX_DIGITS)
		{
			m_fixedFromRight = MAX_DIGITS;
			m_finished = true;

			GLOBALEVENT.publish(Events::Else::DestroyScoreRollEnd());
		}
	}
}

void DestroyScoreUI::DrawSprite()
{
	char scoreBuf[MAX_DIGITS + 1] = {};
	for (int i = 0; i < MAX_DIGITS; ++i)
	{
		int scoreIndex = i - (MAX_DIGITS - m_digitCount);
		int fromRight = MAX_DIGITS - 1 - i;

		if (fromRight < m_fixedFromRight)
		{
			scoreBuf[i] = (scoreIndex < 0) ? '0' : ('0' + m_digits[scoreIndex]);
		}
		else
		{
			scoreBuf[i] = '0' + m_currentRand[i];
		}
	}
	scoreBuf[MAX_DIGITS] = '\0';

	std::string s = "DestroyScore  :     ";
	s += scoreBuf;

	int outlineSize = 7;
	auto sprite = KdFontManager::Instance().CreateFontTexture(5, s, 1, outlineSize);

	int totalWidth = 0;
	for (auto& charData : sprite->GetTexList())
	{
		if (charData->FontTex)
			totalWidth += charData->FontTex->GetInfo().Width;
	}

	int offsetX = -totalWidth / 2;

	for (auto& charData : sprite->GetTexList())
	{
		if (!charData->FontTex) continue;

		if (charData->OutlineTex)
		{
			KdShaderManager::Instance().m_spriteShader.DrawTex(
				charData->OutlineTex,
				m_pos.x + offsetX,
				m_pos.y);
		}

		KdShaderManager::Instance().m_spriteShader.DrawTex(
			charData->FontTex,
			m_pos.x + offsetX,
			m_pos.y);

		offsetX += charData->FontTex->GetInfo().Width;
	}
}