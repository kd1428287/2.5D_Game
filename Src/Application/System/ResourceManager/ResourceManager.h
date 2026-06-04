#pragma once
#include <string>
#include <Framework/Direct3D/KdModel.h>
#include <Framework/Direct3D/Polygon/KdPolygon.h>

class ResourceManager
{
public:
	static ResourceManager& Instance()
	{
		static ResourceManager instance;
		return instance;
	}

	void Init();
	std::shared_ptr<KdModelData> GetModel(const std::string filepath);
	std::shared_ptr<KdPolygon> GetTexture(const std::string filepath);

private:

	void LoadModel(const std::string name, const std::string filepath);
	void LoadTexture(const std::string name, const std::string filepath);

private:
	ResourceManager() {};
	~ResourceManager() {};

	std::shared_ptr<KdModelData> m_model;

	std::map<std::string, std::shared_ptr<KdModelData>> m_models;
	std::map<std::string, KdPolygon> m_polygons;
};

#define RESOURCE ResourceManager::Instance()