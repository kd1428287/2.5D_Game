#pragma once

enum class FadeState
{
	Idle,
	FadeIn,
	FadeOut,
	FadeFinished
};

class FadeManager
{
public:
	static FadeManager& Instance()
	{
		static FadeManager instance;
		return instance;
	}

	void Init(int w, int h);
	void Update(float dt);
	void DrawSprite();

	// フェード状態の問い合わせ
	bool IsFading()      const { return m_state == FadeState::FadeIn || m_state == FadeState::FadeOut; }
	bool IsFadeIn()      const { return m_state == FadeState::FadeIn; }
	bool IsFadeOut()     const { return m_state == FadeState::FadeOut; }
	bool IsFadeFinished() const { return m_state == FadeState::FadeFinished; }

	// 外部から直接フェードを開始できる口を用意（イベント経由以外でも使える）
	void StartFadeIn() { RequestState(FadeState::FadeIn); }
	void StartFadeOut() { RequestState(FadeState::FadeOut); }

	// フェード速度をまとめて設定
	void SetFadeInSpeed(float speed) { m_fadeInSpeed = speed; }
	void SetFadeOutSpeed(float speed) { m_fadeOutSpeed = speed; }

private:
	void UpdateFadeIn(float dt);
	void UpdateFadeOut(float dt);

	// 状態遷移リクエスト（現在の状態と異なる場合のみ受け付ける）
	void RequestState(FadeState next);

	std::shared_ptr<KdTexture> m_texture;

	float      m_fadeScale = 0.f;
	float      m_fadeInSpeed = 2.0f;   // FadeIn の速度
	float      m_fadeOutSpeed = 4.0f;   // FadeOut の速度（旧: fadeSpeed * 2 を明示化）
	FadeState  m_state = FadeState::Idle;
	FadeState  m_nextState = FadeState::Idle;

	int m_width = 0;   // 修正: int に 0.f は型不一致
	int m_height = 0;

	ScopedSubscriber m_toResultSub = {};
	ScopedSubscriber m_ResultSub = {};
	ScopedSubscriber m_toTitleSub = {};
	ScopedSubscriber m_TitleSub = {};

private:
	FadeManager() = default;
	~FadeManager() = default;
};