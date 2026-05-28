#include "GroundBase.h"

void GroundBase::Init()
{
	m_model = std::make_shared<KdModelData>();
	m_model->Load("Asset/Models/");
}

void GroundBase::DrawLit()
{

}
