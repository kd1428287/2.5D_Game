#pragma once

class ResourceManager
{
public:
	static ResourceManager& Instance()
	{
		static ResourceManager instance;
		return instance;
	}

	void Init();

private:

	void LoadModel(const std::string name, const std::string fileName);
	void LoadTexture(const std::string name, const std::string fileName);

private:
	ResourceManager() {};
	~ResourceManager() {};

	std::map<std::string, KdModelData> m_modelMap;
	std::map<std::string, KdPolygon> m_polygonMap;
};