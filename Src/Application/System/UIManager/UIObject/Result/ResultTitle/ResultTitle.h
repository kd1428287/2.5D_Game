#pragma once
#include "../../UIObject.h"

class ResultTitleUI : public UIObject
{
public:
	ResultTitleUI() {};
	~ResultTitleUI()override {};

	void Init()override
	{
		m_pos = { 0,250,0 };
		m_scale = 2.0f;
	}
	void Update(float dt)override
	{

	}
	void DrawSprite()override
	{
		std::string s = "Score";
		int outlineSize = 5;
		auto sprite = KdFontManager::Instance().CreateFontTexture(5, s, 1, outlineSize);

		// 全体幅を計算して中央揃え
		int totalWidth = 0;
		for (auto& charData : sprite->GetTexList())
		{
			if (charData->FontTex)
				totalWidth += charData->FontTex->GetInfo().Width * m_scale;
		}

		int offsetX = -totalWidth / 2;

		for (auto& charData : sprite->GetTexList())
		{
			if (!charData->FontTex) continue;
			Math::Matrix mat =
				Math::Matrix::CreateScale(m_scale) *
				Math::Matrix::CreateTranslation(m_pos + Math::Vector3(offsetX, 0, 0));
			KdShaderManager::Instance().m_spriteShader.SetMatrix(mat);

			if (charData->OutlineTex)
			{
				KdShaderManager::Instance().m_spriteShader.DrawTex(
					charData->OutlineTex, 0, 0);
			}

			KdShaderManager::Instance().m_spriteShader.DrawTex(
				charData->FontTex, 0, 0);

			offsetX += charData->FontTex->GetInfo().Width * m_scale;
		}
		KdShaderManager::Instance().m_spriteShader.SetMatrix(Math::Matrix::Identity);
	}

private:
	
};