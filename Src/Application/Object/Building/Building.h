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
	Building,
	HouseCountry,
	House,
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

	Building(Math::Vector3 pos) :m_pos(pos)
	{
	/*	m_mWorld =
			Math::Matrix::CreateTranslation(pos);*/
	};

	Building(Math::Vector3 pos, int level, int type, float dir) :
		m_pos(pos), m_breakLevel(level), m_type((BuildingType)type), m_dir(dir)
	{
	};

	~Building()override {};

	void Init()override;
	void Update(float dt)override;

	void GenerateDepthMapFromLight()override;
	void DrawLit()override;

	void SetPos(Math::Vector3 pos);
	void SetDir(float dir);

	float GetDir() { return m_dir; }
	BuildingState GetState() { return m_state; }

protected:
	void Break(KdCollider::CollisionResult result);

	void SetBreakLevel(int level);
	void SetModel(BuildingType type);

	std::shared_ptr<KdModelData> m_model = nullptr;
	std::shared_ptr<KdModelWork> m_fragmentModel = nullptr;
	BuildingState m_state = BuildingState::Unbroken;
	BuildingType m_type = BuildingType::HouseCountry;

	std::vector<FragmentVelocity> fragVelocities;

	Math::Vector3 m_pos;
	float m_dir = 0.f;	// 正面方向(0で-z方向)
	float m_scale = 1.f;

	float m_breakCount = 0.0f;
	int m_breakLevel = 4;

	float m_buildingDissolve = 0.f;

	Math::Color m_color = { 1.f,1.f,1.f,1.f };
};