#pragma once
#include "../../UIObject.h"

class CardBoard : public UIObject
{
public:
	CardBoard() {};
	CardBoard(Math::Vector3 pos, float dir = 180.f)
		: m_pos(pos), m_dir(dir) {};
	~CardBoard() {};

	void Init()override;
	void Update(float dt)override;
	void GenerateDepthMapFromLight()override;
	void DrawLit()override;

private:
	std::shared_ptr<KdModelData> m_model = nullptr;

	Math::Vector3 m_pos;
	float m_dir = 180.f;		// 初期向き[deg]
	float m_rotAngle = 0.f;		// 現在の累積回転角度[deg]

	static constexpr float ROT_SPEED = 90.f;	// 回転速度[deg/s]
};