#include "UIManager.h"
#include "UIObject/UIObject.h"
#include "UIObject/Title/TitleText/TitleTextUI.h"
#include "UIObject/Game/Meter/MeterUI.h"
#include "UIObject/Game/Time/TimeUI.h"
#include "UIObject/Result/DeliveryScore/DeliveryScoreUI.h"
#include "UIObject/Result/DestroyScore/DestroyScoreUI.h"
#include "UIObject/Result/Back/BackUI.h"
#include "UIObject/Result/ResultText/ResultTextUI.h"
#include "UIObject/Result/ResultTitle/ResultTitle.h"

void UIManager::Init()
{
	CreateUI(UIPaturn::Title);

	// Title シーン開始イベント (実装時に適切なイベント型へ変更)
	 m_titleSub = GLOBALEVENT.subscribe<Events::Else::TitleBegin>([this](const Events::Else::TitleBegin& e)
	 	{
	 		CreateUI(UIPaturn::Title);
	 	});

	m_gameSub = GLOBALEVENT.subscribe<Events::Else::GameStart>([this](const Events::Else::GameStart& e)
		{
			CreateUI(UIPaturn::Game);
		});

	m_resultSub = GLOBALEVENT.subscribe<Events::Else::ResultBegin>([this](const Events::Else::ResultBegin& e)
		{
			CreateUI(UIPaturn::Result);
		});
}

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

void UIManager::GenerateDepthMapFromLight()
{
	for (auto& obj : m_UIobjList)
	{
		obj->GenerateDepthMapFromLight();
	}
}

void UIManager::DrawLit()
{
	for (auto& obj : m_UIobjList)
	{
		obj->DrawLit();
	}
}

void UIManager::Release()
{
	m_UIobjList.clear();
}

void UIManager::CreateUI(UIPaturn paturn)
{
	// パターン切替前に前シーンのUIを必ず破棄する
	Release();

	std::shared_ptr<UIObject> ui;
	switch (paturn)
	{
	case UIPaturn::Title:
		ui = std::make_shared<TitleTextUI>();
		ui->Init();
		m_UIobjList.push_back(ui);

		break;
	case UIPaturn::Game:
		ui = std::make_shared<MeterUI>();
		ui->Init();
		m_UIobjList.push_back(ui);
		ui = std::make_shared<TimeUI>();
		ui->Init();
		m_UIobjList.push_back(ui);

		break;
	case UIPaturn::Result:
		/*ui = std::make_shared<BackUI>();
		ui->Init();
		m_UIobjList.push_back(ui);
		ui = std::make_shared<DeliveryScoreUI>();
		ui->Init();
		m_UIobjList.push_back(ui);
		ui = std::make_shared<DestroyScoreUI>();
		ui->Init();
		m_UIobjList.push_back(ui);
		ui = std::make_shared<ResultTextUI>();
		ui->Init();
		m_UIobjList.push_back(ui);
		ui = std::make_shared<ResultTitleUI>();
		ui->Init();
		m_UIobjList.push_back(ui);*/

		ui = std::make_shared<DeliveryScoreUI>();
		ui->Init();
		m_UIobjList.push_back(ui);
		break;
	default:
		break;
	}
}