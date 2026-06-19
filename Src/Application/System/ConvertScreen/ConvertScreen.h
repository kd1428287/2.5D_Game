#pragma once

struct ConvertData
{
	Math::Matrix mat;
	std::weak_ptr<KdTexture> tex;
};

class ConvertScreen
{
public:
	static ConvertScreen& Instance()
	{
		static ConvertScreen instance;
		return instance;
	}

	void RequestConvertScreen(ConvertData data)
	{
		m_list.push_back(data);
	}

	std::vector<ConvertData> AcceptConvertScreen() { 
		auto result = m_list;
		m_list.clear();
		return result;
	};

private:
	ConvertScreen() {};
	~ConvertScreen() {};

	std::vector<ConvertData> m_list;
};