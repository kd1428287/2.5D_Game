#pragma once

struct EffectAnimeParam
{
	// 順番に並んでいるとき
	int startIdx = 0;
	int endIdx = 0;

	// バラバラなとき
	std::vector<int> idxArray;

	float animeIdx = 0.f;
	float cntSpeed = 0.f;

};

class EffectBase : public KdGameObject
{
public:
	EffectBase() {};
	EffectBase(Math::Vector3 pos, float scale = 1.f, bool loop = false) :
		m_pos(pos), m_scale(scale), m_loopFlg(loop) {};
	~EffectBase()override {};

	void Init()override {};
	void Update(float dt)override {};
	void PostUpdate()override {};

	void DrawUnLit()override {};
	void DrawBright()override {};

protected:
	std::shared_ptr<KdSquarePolygon> m_polygon = nullptr;

	float m_anime = 0.f;
	EffectAnimeParam m_animeParam = {};

	Math::Vector3 m_pos;
	float m_scale = 0.f;
	bool m_loopFlg = false;

};