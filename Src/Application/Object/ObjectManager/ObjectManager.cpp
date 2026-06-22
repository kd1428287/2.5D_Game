#include "ObjectManager.h"
#include "Application/main.h"
#include "MapManager/MapManager.h"
#include "../Character/Player/Player.h"
#include "../Building/Building.h"
#include "../Ground/Ground.h"
#include "../SkySphere/SkySphere.h"
#include "../EventObject/DeliveryPoint.h"
#include "../Effect/Exprosion/Exprosion.h"
#include "../Effect/Smoke/Smoke.h"
#include "../Number/Number.h"

void ObjectManager::Init()
{
	
	m_factory["Player"]		= [this](Events::Else::CreateObjectEvent::ObjectParameter param) { return this->CreateObject<Player>(param.m_pos); };
	m_factory["Exprosion"]	= [this](Events::Else::CreateObjectEvent::ObjectParameter param) { return this->CreateObject<Exprosion>(param.m_pos); };
	m_factory["Smoke"]		= [this](Events::Else::CreateObjectEvent::ObjectParameter param) { return this->CreateObject<Smoke>(param.m_pos, param.m_scale, param.m_flg, param.m_float1); };
	m_factory["Number"]		= [this](Events::Else::CreateObjectEvent::ObjectParameter param) { return this->CreateObject<Number>(param.m_pos); };
	m_factory["Player"]		= [this](Events::Else::CreateObjectEvent::ObjectParameter param) { return this->CreateObject<Player>(param.m_pos); };

	m_createSub = GLOBALEVENT.subscribe<Events::Else::CreateObjectEvent>([this](const Events::Else::CreateObjectEvent& e) {

		auto it = m_factory.find(e.m_objectType);

		if (it != m_factory.end()) {
			it->second(e.m_param); // マップに登録された関数を起動
		}
		});
}

void ObjectManager::PreUpdate()
{
	auto it = m_objList.begin();

	while (it != m_objList.end())
	{
		if ((*it)->IsExpired())	// IsExpired() ・・・ 無効ならtrue
		{
			// 無効なオブジェクトをリストから削除
			it = m_objList.erase(it);
		}
		else
		{
			++it;	// 次の要素へイテレータを進める
		}
	}

	// ↑の後には有効なオブジェクトだけのリストになっている

	// 追加待ちリストを追加
	for (auto obj : m_nextObjList)
	{
		m_objList.push_back(obj);
	}

	for (auto& obj : m_objList)
	{
		obj->PreUpdate();
	}
}

void ObjectManager::Update(float dt)
{
	for (auto& obj : m_objList)
	{
		obj->Update(dt);
	}
}

void ObjectManager::PostUpdate()
{
	for (auto& obj : m_objList)
	{
		obj->PostUpdate();
	}
}

void ObjectManager::GenerateDepthMapFromLight()
{
	for (auto& obj : m_objList)
	{
		obj->GenerateDepthMapFromLight();
	}
}

void ObjectManager::PreDraw()
{
	for (auto& obj : m_objList)
	{
		obj->PreDraw();
	}
}

void ObjectManager::DrawLit()
{
	for (auto& obj : m_objList)
	{
		obj->DrawLit();
	}
}

void ObjectManager::DrawUnLit()
{
	for (auto& obj : m_objList)
	{
		obj->DrawUnLit();
	}
}

void ObjectManager::DrawEffect()
{
	for (auto& obj : m_objList)
	{
		obj->DrawEffect();
	}
}

void ObjectManager::DrawBright()
{
	for (auto& obj : m_objList)
	{
		obj->DrawBright();
	}
}

void ObjectManager::DrawSprite()
{
	for (auto& obj : m_objList)
	{
		obj->DrawSprite();
	}
}

void ObjectManager::DrawDebug()
{
	for (auto& obj : m_objList)
	{
		obj->DrawDebug();
	}
}