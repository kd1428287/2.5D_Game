#pragma once

class AudioManager
{
public:
	static AudioManager& Instance()
	{
		static AudioManager instance;
		return instance;
	}

	void Init();
private:
	AudioManager() {};
	~AudioManager() {};

	std::vector<ScopedSubscriber> m_subscriber;
};