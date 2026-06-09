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
			//Math::Matrix::CreateScale(0.1f) *
			Math::Matrix::CreateTranslation(pos);
	};
	Building(Math::Vector3 pos, float level) :m_breakLevel(level)
	{
		m_mWorld =
			//Math::Matrix::CreateScale(0.1f) *
			Math::Matrix::CreateTranslation(pos);
	};
	~Building()override {};

	void Init()override;
	void Update(float dt)override;
	
	void GenerateDepthMapFromLight()override;
	void DrawLit()override;

protected:
	void Break(KdCollider::CollisionResult result);

	std::shared_ptr<KdModelData> m_model = nullptr;
	std::shared_ptr<KdModelWork> m_fragmentModel = nullptr;
	BuildingState m_state = BuildingState::Unbroken;

	std::vector<FragmentVelocity> fragVelocities;

	float m_breakCount = 0.0f;
	int m_breakLevel = 4;
};