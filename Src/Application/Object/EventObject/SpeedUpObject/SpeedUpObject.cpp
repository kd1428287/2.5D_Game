#include "SpeedUpObject.h"

void SpeedUpObject::Init()
{
	m_model = std::make_shared<KdModelData>();
	m_model = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/Effect/SpeedUp.gltf");
}

void SpeedUpObject::Update(float dt)
{}

void SpeedUpObject::PostUpdate()
{}

void SpeedUpObject::GenerateDepthMapFromLight()
{
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}

void SpeedUpObject::DrawLit()
{
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}
