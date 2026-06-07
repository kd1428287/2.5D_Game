#pragma once

class SkySphere : public KdGameObject
{
public:
	SkySphere() {};
	~SkySphere()override {};

	void Init()override;

	void DrawUnLit()override;

private:
	std::shared_ptr<KdModelData> m_model = nullptr;
};