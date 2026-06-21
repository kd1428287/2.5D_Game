#include "ResultScene.h"
#include "../SceneManager.h"

#include "Application/System/InputManager/InputManager.h"
#include "Application/Object/ObjectManager/ObjectManager.h"

void ResultScene::Init()
{
	BaseScene::Init();

	// リザルト表示開始を通知（FadeManager が FadeOut を開始する）
	GLOBALEVENT.publish(Events::Else::ResultBegin());

	m_cnt = 0.f;
	m_state = InScene::GameToResult;
}

void ResultScene::Event(float dt)
{
	// リターンキーでタイトルへ戻る
	if (InputManager::Instance().IsTriggered(VK_RETURN))
	{
		if (m_state == InScene::Result)
		{
			// タイトルへの暗転開始を通知（FadeManager が FadeIn を開始する）
			GLOBALEVENT.publish(Events::Else::ResultEnd());
			GLOBALEVENT.publish(Events::Else::ResultToTitleBegin());
			m_state = InScene::ResultToTitle;
		}
	}

	if (m_state == InScene::ResultToTitle)
	{
		m_cnt += dt;

		if (m_cnt > 1.5f)
		{
			SceneManager::Instance().SetNextScene(SceneManager::SceneType::Game);
			GLOBALEVENT.publish(Events::Else::ResultToTitleEnd());
		}
	}

	if (m_state == InScene::GameToResult)
	{
		m_cnt += dt;

		if (m_cnt > 0.5f)
		{
			m_state = InScene::Result;
			m_cnt = 0;
			GLOBALEVENT.publish(Events::Else::DeliveryScoreRollBegin());
		}
	}

}