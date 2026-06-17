#pragma once

class SpeedUpObject : public KdGameObject
{
public:
	SpeedUpObject() {};
	SpeedUpObject(Math::Vector3 pos) : m_pos(pos) { m_mWorld = Math::Matrix::CreateTranslation(pos); };
	~SpeedUpObject()override {};

	void Init()override;
	void Update(float dt)override;
	void PostUpdate()override;

	void GenerateDepthMapFromLight()override;
	void DrawLit()override;

private:
	std::shared_ptr<KdModelData> m_model;
	float m_rotateRatio = 0.f;
	Math::Vector3 m_pos;

	float m_durationContact = 0.0f;

	bool m_isDelivered = false;

	float m_radius = 0.13f;

	// 浮遊・回転用
	float m_floatTime = 0.0f;          // 浮遊アニメーション用タイマー
	float m_floatAmplitude = 0.05f;        // 上下の振れ幅（単位: m）
	float m_floatSpeed = 2.0f;        // 浮遊の周期速度（rad/s）
	float m_rotateSpeed = 90.0f;       // Y軸回転速度（度/s）
	float m_rotateAngle = 0.0f;        // 現在のY軸回転角度（度）

};