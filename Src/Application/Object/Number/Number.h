#pragma once

class Number : public KdGameObject
{
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

protected:
	// m_pos / m_dir / m_scale / m_animOffset / m_animScale から m_mWorld を再構築する
	// アニメーション演出を行う派生クラスはこれを呼んで行列を更新する
	void RebuildMatrix()
	{
		m_mWorld =
			Math::Matrix::CreateScale(m_scale * m_animScale) *
			Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_dir + m_animExtraDir)) *
			Math::Matrix::CreateTranslation(m_pos + m_animOffset);
	}

	std::shared_ptr<KdModelData> m_model = nullptr;

	int m_number = 0;
	Math::Vector3 m_pos;
	float m_dir = 0.f;	// 正面方向(0で-z方向)
	float m_scale = 1.f;

	// ---- アニメーション用：派生クラスが書き換えて RebuildMatrix() を呼ぶ ----
	Math::Vector3 m_animOffset = {};   // 位置オフセット
	float         m_animScale = 1.f;  // スケール倍率
	float         m_animExtraDir = 0.f;  // 追加回転角度[deg]
};