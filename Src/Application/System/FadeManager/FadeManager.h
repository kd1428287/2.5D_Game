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

private:
	void UpdateFadeIn(float dt);
	void UpdateFadeOut(float dt);

	std::shared_ptr<KdTexture> m_texture;

	float m_fadeScale = 0.f;
	float m_fadeSpeed = 2.0f;
	FadeState m_state = FadeState::Idle;

	int m_width = 0.f;
	int m_height = 0.f;

	ScopedSubscriber m_toResultSub = {};
	ScopedSubscriber m_toTitleSub = {};

private:
	FadeManager() {};
	~FadeManager() {};
};