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

		struct GetSpeedUp : public Event
		{
			std::weak_ptr<KdGameObject> m_me;
			GetSpeedUp(const std::shared_ptr<KdGameObject>& me) :m_me(me) {};
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
			float speedLevel = 0.f;
			HitResult(const HitResultType& _type, float speed) :type(_type), speedLevel(speed) {};
		};

		struct DeliveryPointBegin : public Event
		{
			std::weak_ptr<KdGameObject> m_me;
			DeliveryPointBegin(const std::shared_ptr<KdGameObject>& me) :m_me(me) {};
		};

		struct DeliveryPointEnd : public Event
		{
			std::weak_ptr<KdGameObject> m_me;
			DeliveryPointEnd(const std::shared_ptr<KdGameObject>& me) :m_me(me) {};
		};

		struct DeliveryPointCompleted :public Event
		{
			std::weak_ptr<KdGameObject> m_me;
			DeliveryPointCompleted(const std::shared_ptr<KdGameObject>& me) :m_me(me) {};
		};

		struct DeliveryPointDeleted :public Event
		{
			std::weak_ptr<KdGameObject> m_me;
			DeliveryPointDeleted(const std::shared_ptr<KdGameObject>& me) :m_me(me) {};
		};
	}

	namespace Else
	{
		struct TitleToGameBegin : public Event {};
		struct TitleToGameEnd : public Event {};

		struct GameStart : public Event
		{};

		struct GameEnd : public Event
		{};

		struct CreateObject : public Event
		{
			enum class ObjectType
			{
				
			};
			std::shared_ptr<KdGameObject> obj;
		};

		struct CreateParticle : public Event
		{
			enum class ParticleType
			{

			};
			ParticleType m_type;
			Math::Vector3 m_pos;
			float m_scale = 0.f;
			bool m_loopFlg = false;
			CreateParticle(
				ParticleType type,
				Math::Vector3 pos,
				float scale,
				bool loopFlg) :
				m_type(type), m_pos(pos), m_scale(scale), m_loopFlg(loopFlg) {};
		};
	}
}