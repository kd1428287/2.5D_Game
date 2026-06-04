#include "ResourceManager.h"

void ResourceManager::Init()
{
	m_model = std::make_shared<KdModelData>();
	m_model->Load("Asset/Models/map_tiles/map_grass.gltf");
}

std::shared_ptr<KdModelData> ResourceManager::GetModel(const std::string filepath)
{
	auto it = m_models.find(filepath);

	if (it != m_models.end()) {
		return it->second;
	}
	else {
		std::shared_ptr<KdModelData> newModel = std::make_shared<KdModelData>();
		
		if (newModel->Load(filepath))
		{
			m_models[filepath] = newModel;

			return newModel;
		}
		else 
		{
			return nullptr;
		}
	}
}

std::shared_ptr<KdPolygon> ResourceManager::GetTexture(const std::string filepath)
{
	return std::shared_ptr<KdPolygon>();
}

void ResourceManager::LoadModel(const std::string name, const std::string filepath)
{
}

void ResourceManager::LoadTexture(const std::string name, const std::string filepath)
{}
