#pragma once

enum class BuildingState
{
	None = 0,
	Unbroken,
	Broken,
	Erase,
};

enum class BuildingType
{

};

struct FragmentVelocity {
	Math::Vector3 position;			// 現在の位置
	float speed = 10.0f;			// 速度
	Math::Vector3 direction;		// 方向
	Math::Vector3 rotation;			// 現在の回転角
	Math::Vector3 angularVelocity;	// 回転速度
};

class Building : public KdGameObject
{
public:
	Building() {};

	Building(Math::Vector3 pos) 
	{
		m_mWorld =
			Math::Matrix::CreateTranslation(pos);
	};

	Building(Math::Vector3 pos, int level) :m_breakLevel(level)
	{
		float scale = 1.f;
		if (level == 6)scale = 1.3f;
		m_mWorld =
			Math::Matrix::CreateScale(scale) *
			Math::Matrix::CreateTranslation(pos);
	};

	~Building()override {};

	void Init()override;
	void Update(float dt)override;
	
	void GenerateDepthMapFromLight()override;
	void DrawLit()override;

	BuildingState GetState() { return m_state; }

protected:
	void Break(KdCollider::CollisionResult result);

	void SetBreakLevel(int level);

	std::shared_ptr<KdModelData> m_model = nullptr;
	std::shared_ptr<KdModelWork> m_fragmentModel = nullptr;
	BuildingState m_state = BuildingState::Unbroken;

	std::vector<FragmentVelocity> fragVelocities;

	float m_breakCount = 0.0f;
	int m_breakLevel = 4;

	float m_buildingDissolve = 0.f;

	Math::Color m_color = { 1.f,1.f,1.f,1.f };
};