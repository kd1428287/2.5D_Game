#include "TimeUI.h"
#include "../../../../Reader/Reader.h"

void TimeUI::Init()
{
	m_time = 0.f;
	m_pos = {-600,300};
}

void TimeUI::Update(float dt)
{
	float elapsed = Reader::Instance().ReadTime();
	m_time = 90.0f - elapsed;
	if (m_time < 0.0f) m_time = 0.0f;
}

void TimeUI::DrawSprite()
{
	int totalSec = static_cast<int>(m_time);
	int minutes = totalSec / 60;
	int seconds = totalSec % 60;

	char buf[16];
	std::snprintf(buf, sizeof(buf), "%d:%02d", minutes, seconds);
	std::string s = buf;
	int outlineSize = 7;
	auto sprite = KdFontManager::Instance().CreateFontTexture(5, s, 1, outlineSize);

	// 1文字ずつテクスチャが格納されているので、順番に描画する
	auto& texList = sprite->GetTexList();
	
	int offsetX = 0;
	for (auto& charData : sprite->GetTexList())
	{
		if (charData->OutlineTex)
		{
			KdShaderManager::Instance().m_spriteShader.DrawTex(charData->OutlineTex, m_pos.x + offsetX , m_pos.y);
		}

		KdShaderManager::Instance().m_spriteShader.DrawTex(charData->FontTex, m_pos.x + offsetX, m_pos.y);

		offsetX += charData->FontTex->GetInfo().Width;
	}
}