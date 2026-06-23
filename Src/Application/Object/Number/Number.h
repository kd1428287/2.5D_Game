#pragma once

class Number : public KdGameObject
{
	friend class DeliveryScoreUI;

public:
	Number() {};
	~Number()override {};
	Number(Math::Vector3 pos, int num = 0, float dir = 180.f) :
		m_pos(pos), m_number(num), m_dir(dir)
	{
		m_mWorld =
			Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(dir)) *
			Math::Matrix::CreateTranslation(pos);
	};

	void Init()override;
	void Update(float dt)override;

	void GenerateDepthMapFromLight()override;
	void DrawLit()override;

	void SetPos(Math::Vector3 pos) { m_pos = pos; }
	void SetDir(float dir) { m_dir = dir; }
	void SetNumber(int num);

	float GetDir() { return m_dir; }

	Math::Vector3 GetPos() const override { return m_pos; }
	void SetMatrix(const Math::Matrix& mat) { m_mWorld = mat; }

private:
	std::shared_ptr<KdModelData> m_model = nullptr;

	int m_number = 0;
	Math::Vector3 m_pos;
	float m_dir = 0.f;	// 正面方向(0で-z方向)
	float m_scale = 1.f;
};