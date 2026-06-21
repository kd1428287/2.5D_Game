#pragma once
#include "../../UIObject.h"

class BackUI : public UIObject
{
public:
	BackUI() {};
	~BackUI()override {};

	void Init()
	{
		m_texture = KdAssets::Instance().m_textures.GetData("Asset/Textures/UI/Result/back.png");
	}

	void DrawSprite()
	{
		KdShaderManager::Instance().m_spriteShader.DrawTex(m_texture, 0, 0);
	}
};