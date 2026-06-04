#pragma once

class Event {
public:
	virtual ~Event() = default;
};

namespace Event
{
	namespace Player
	{
		struct ChangeSpeedLevel : public Event
		{
			int level = 0;
		};
	}
}