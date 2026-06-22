#include "Number.h"

void Number::Init()
{
	m_model = std::make_shared<KdModelData>();
	SetNumber(m_number);
}

void Number::Update(float dt)
{
}

void Number::GenerateDepthMapFromLight()
{
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}

void Number::DrawLit()
{
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}

void Number::SetNumber(int num)
{
	m_number = std::clamp(num, 0, 9);
	std::string s = "Asset/Models/Effect/Number/number-" + std::to_string(m_number) + ".glb";
	m_model = KdAssets::Instance().m_modeldatas.GetData(s);
}