#include "FadeManager.h"

void FadeManager::Init(int w, int h)
{
	// make_shared してすぐ上書きしていた無駄なアロケーションを削除
	m_texture = KdAssets::Instance().m_textures.GetData("Asset/Textures/System/dot.png");

	m_fadeScale = 1.f;
	m_fadeInSpeed = 2.f;
	m_fadeOutSpeed = 4.f;   // 旧コードの "m_fadeSpeed * 2" を明示的な変数として管理
	m_state = FadeState::FadeFinished;
	m_nextState = FadeState::FadeFinished;

	m_width = w;
	m_height = h;

	// ゲーム → リザルト: 画面を暗転（FadeIn）
	m_toResultSub = GLOBALEVENT.subscribe<Events::Else::GameToResultBegin>(
		[this](const Events::Else::GameToResultBegin&)
		{
			StartFadeIn();
		});

	// リザルト開始: 画面を明転（FadeOut）
	m_ResultSub = GLOBALEVENT.subscribe<Events::Else::ResultBegin>(
		[this](const Events::Else::ResultBegin&)
		{
			StartFadeOut();
		});

	// リザルト → タイトル: 画面を暗転（FadeIn）
	m_toTitleSub = GLOBALEVENT.subscribe<Events::Else::ResultToTitleBegin>(
		[this](const Events::Else::ResultToTitleBegin&)
		{
			StartFadeIn();
		});

	// タイトル開始: 画面を明転（FadeOut）
	m_TitleSub = GLOBALEVENT.subscribe<Events::Else::TitleBegin>(
		[this](const Events::Else::TitleBegin&)
		{
			StartFadeOut();
		});
}

void FadeManager::Update(float dt)
{
	// 待機中に新しいリクエストがあれば状態を切り替え
	if (m_state == FadeState::Idle || m_state == FadeState::FadeFinished)
	{
		if (m_nextState != FadeState::Idle)
		{
			m_state = m_nextState;
			m_nextState = FadeState::Idle;
		}
	}

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
	// 完全に透明 or 待機中は描画しない
	if (m_state == FadeState::Idle || m_fadeScale <= 0.f) return;

	const Math::Rectangle rect = { 0, 0, m_width, m_height };
	const Math::Color     color = { 0.f, 0.f, 0.f, m_fadeScale };
	const Math::Matrix    mat = Math::Matrix::CreateScale(
		{ static_cast<float>(m_width), static_cast<float>(m_height), 1.f });

	auto& shader = KdShaderManager::Instance().m_spriteShader;
	shader.SetMatrix(mat);
	shader.DrawTex(m_texture, 0, 0, &rect, &color);
	shader.SetMatrix(Math::Matrix::Identity);
}

// ─── private ────────────────────────────────────────────────────────────────

void FadeManager::RequestState(FadeState next)
{
	m_nextState = next;
}

void FadeManager::UpdateFadeIn(float dt)
{
	m_fadeScale = std::lerp(m_fadeScale, 1.f, m_fadeInSpeed * dt);

	// 浮動小数点誤差を吸収して確実に 1.0 で止める
	if (m_fadeScale >= 0.99f)
	{
		m_fadeScale = 1.f;
		m_state = FadeState::FadeFinished;
	}
}

void FadeManager::UpdateFadeOut(float dt)
{
	m_fadeScale = std::lerp(m_fadeScale, 0.f, m_fadeOutSpeed * dt);

	// 浮動小数点誤差を吸収して確実に 0.0 で止める
	if (m_fadeScale <= 0.01f)
	{
		m_fadeScale = 0.f;
		m_state = FadeState::Idle;
	}
}