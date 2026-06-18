#include "FadeManager.h"

void FadeManager::Init(int w,int h)
{
	m_texture = std::make_shared<KdTexture>();
	m_texture = KdAssets::Instance().m_textures.GetData("Asset/Textures/System/dot.png");

	m_fadeScale = 0.f;
	m_fadeSpeed = 2.0f;

	m_width = w;
	m_height = h;
}

void FadeManager::Update(float dt)
{
	switch (m_state)
	{
	case FadeState::FadeIn:
		UpdateFadeIn(dt);
		break;
	case FadeState::FadeOut:
		UpdateFadeOut(dt);
		break;
	default:
		break;
	}
}

void FadeManager::DrawSprite()
{
	if (m_fadeScale <= 0.f)return;

	Math::Rectangle rect = { 0,0,m_width,m_height };
	Math::Color color = { 1, 1, 1, m_fadeScale };
	KdShaderManager::Instance().m_spriteShader.DrawTex(m_texture, 0, 0, &rect, &color);
}

void FadeManager::UpdateFadeIn(float dt)
{
	m_fadeScale = std::lerp(m_fadeScale, 1.f, m_fadeSpeed * dt);
	if (m_fadeScale >= 1.f)m_state = FadeState::FadeFinished;
}

void FadeManager::UpdateFadeOut(float dt)
{
	m_fadeScale = std::lerp(m_fadeScale, 0.f, m_fadeSpeed * dt);
	if (m_fadeScale <= 0.f)m_state = FadeState::Idle;
}
