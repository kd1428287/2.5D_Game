#pragma once

class EffectBase : public KdGameObject
{
public:
	EffectBase() {};
	EffectBase(Math::Vector3 pos, float scale, bool loop) :
		m_pos(pos), m_scale(scale), m_loopFlg(loop) {};
	~EffectBase()override {};

	void Init()override;
	void Update(float dt)override;
	void PostUpdate()override;

	void DrawUnLit()override;

protected:
	std::shared_ptr<KdPolygon> m_polygon = nullptr;

	float m_anime = 0.f;

	Math::Vector3 m_pos;
	float m_scale = 0.f;
	bool m_loopFlg = false;

};