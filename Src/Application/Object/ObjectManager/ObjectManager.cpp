#include "ObjectManager.h"
#include "Application/main.h"

void ObjectManager::Init()
{}

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
