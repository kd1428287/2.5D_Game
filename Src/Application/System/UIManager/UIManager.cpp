#include "UIManager.h"
#include "UIObject/UIObject.h"

void UIManager::Init()
{}

void UIManager::Update(float dt)
{}

void UIManager::DrawSprite()
{}

void UIManager::CreateUI(UIPaturn paturn)
{
	switch (paturn)
	{
	case UIPaturn::Title:
		break;
	case UIPaturn::Game:
		break;
	case UIPaturn::Result:
		break;
	default:
		break;
	}
}
