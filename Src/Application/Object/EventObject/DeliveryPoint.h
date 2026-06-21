#pragma once

class Building;



class DeliveryPoint :public KdGameObject
{
public:
	DeliveryPoint() {};
	DeliveryPoint(const std::shared_ptr<Building>& obj, Math::Vector3 pos, float radius);
	~DeliveryPoint()override {};

	void Init()override;
	void Update(float dt)override;
	void PostUpdate()override;
	void DrawUnLit()override;

	void GenerateDepthMapFromLight()override;
	void DrawLit()override;
	void DrawBright()override;
	void DrawSprite()override;

	void SetParent(const std::shared_ptr<Building>& obj) { m_wpParent = obj; }

private:
	void UpdateHitCollision(const std::vector<std::shared_ptr<KdGameObject>>& objList);

	std::shared_ptr<KdModelData> m_model;
	std::shared_ptr<KdTexture> m_texture;

	Math::Vector3 m_arrowDistance = { 0.f,0.5f,0.f };

	std::weak_ptr<Building> m_wpParent;
	float m_radius = 0.0f;
	float m_deltaTime = 0.0f;

	// 放物線アニメーション用
	Math::Vector3 m_cardboardPos;       // 開始位置（既存）
	Math::Vector3 m_cardboardWorldPos;  // 実際の描画位置（ワールド座標）
	float m_cardboardScale = 0.2f;		// 描画スケール（アニメーション用）
	float m_animTime = 0.0f;			// 経過時間
	float m_animDuration = 1.f;			// アニメーション総時間（秒）
	float m_arcHeight = 0.4f;			// 放物線の頂点の高さ
	bool  m_isAnimating = false;		// アニメーション中フラグ

	//static const float MAX_DURATION = 100.0f;
	float m_durationContact = 0.0f;

	bool m_isDelivered = false;
	bool m_isSpriteDraw = false;

	ScopedSubscriber m_startSub;
};