#pragma once

class Event {
public:
	virtual ~Event() = default;
};

namespace Events
{
	namespace Player
	{
		struct ChangeSpeedLevel : public Event
		{
			int level = 0;

			ChangeSpeedLevel(int i) :level(i) {}
		};

		struct OnHit : public Event
		{
			std::weak_ptr<KdGameObject> m_me;
			std::weak_ptr<KdGameObject> m_other;
			KdCollider::CollisionResult m_result;
			OnHit(const std::shared_ptr<KdGameObject>& me, const std::shared_ptr<KdGameObject>& other,
				KdCollider::CollisionResult result) : m_me(me), m_other(other), m_result(result) {};
		};

		struct HitResult : public Event
		{
			enum class HitResultType {
				Destroyed, // 破壊成功
				Bounced,   // 破壊失敗・跳ね返り
				Ignored    // 対象外
			};

			HitResultType type;
		};
	}
}