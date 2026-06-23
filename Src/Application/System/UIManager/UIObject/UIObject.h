#pragma once

class UIObject : public KdGameObject
{
public:
	UIObject() {};
	~UIObject()override {};

	void Init()override {};
	void Update(float dt)override {};
	void DrawSprite()override {};

protected:
	std::shared_ptr<KdTexture> m_texture;
	Math::Vector3 m_pos;
	float m_scale = 0.f;
};