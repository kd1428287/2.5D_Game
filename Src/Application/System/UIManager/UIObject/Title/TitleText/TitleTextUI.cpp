#include "TitleTextUI.h"
#include <cmath>
#include <string>

void TitleTextUI::Init()
{
	m_time = 0.f;
	m_pos = m_basePos;

	m_startSub = GLOBALEVENT.subscribe<Events::Else::TitleToGameBegin>
		([this](const Events::Else::TitleToGameBegin& e)
			{
				m_isPressed = true;
			}
		);
}

void TitleTextUI::Update(float dt)
{
	m_time += dt;

	if (m_isPressed)
	{
		// フェードアウト速度: 1.5f → 約0.67秒で0になる
		m_fadeAlpha -= dt * 1.5f;
		if (m_fadeAlpha < 0.f) m_fadeAlpha = 0.f;
	}
}

void TitleTextUI::DrawSprite()
{
	// フェードアウト完了後は描画しない
	if (m_isPressed && m_fadeAlpha <= 0.f) return;

	// --- 上下ボブ ---
	float bobY = m_bobAmp * std::sin(m_bobSpeed * m_time);

	// --- 明滅アルファ (sin を 0〜1 に正規化) ---
	float t = (std::sin(m_blinkSpeed * m_time) + 1.0f) * 0.5f;
	float blinkAlpha = m_alphaMin + (m_alphaMax - m_alphaMin) * t;

	// m_isPressed後はフェードアウト値を乗算して暗くしていく
	float alpha = blinkAlpha * m_fadeAlpha;

	std::string s = "Press Enter";
	int outlineSize = 5;
	auto sprite = KdFontManager::Instance().CreateFontTexture(5, s, 1, outlineSize);

	// 全体幅を計算して中央揃え
	int totalWidth = 0;
	for (auto& charData : sprite->GetTexList())
	{
		if (charData->FontTex)
			totalWidth += charData->FontTex->GetInfo().Width;
	}

	int offsetX = -totalWidth / 2;
	float drawY = m_basePos.y + bobY;

	for (auto& charData : sprite->GetTexList())
	{
		if (!charData->FontTex) continue;

		Math::Color color = { 1.f, 1.f, 1.f, alpha };
		if (charData->OutlineTex)
		{
			KdShaderManager::Instance().m_spriteShader.DrawTex(
				charData->OutlineTex,
				m_basePos.x + offsetX,
				drawY,
				nullptr,
				&color);
		}

		KdShaderManager::Instance().m_spriteShader.DrawTex(
			charData->FontTex,
			m_basePos.x + offsetX,
			drawY,
			nullptr,
			&color);

		offsetX += charData->FontTex->GetInfo().Width;
	}
}