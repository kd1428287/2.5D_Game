#include "UIManager.h"
#include "UIObject/UIObject.h"
#include "UIObject/Game/Meter/Meter.h"

void UIManager::Init()
{}

void UIManager::Update(float dt)
{
	for (auto& obj : m_UIobjList)
	{
		obj->Update(dt);
	}
}

void UIManager::DrawSprite()
{
	for (auto& obj : m_UIobjList)
	{
		obj->DrawSprite();
	}
}

void UIManager::CreateUI(UIPaturn paturn)
{
	std::shared_ptr<UIObject> ui;
	switch (paturn)
	{
	case UIPaturn::Title:
		

		break;
	case UIPaturn::Game:
		ui = std::make_shared<MeterUI>();
		ui->Init();
		m_UIobjList.push_back(ui);

		break;
	case UIPaturn::Result:
		break;
	default:
		break;
	}
}
