#pragma once
#include <typeindex>
//#include <Framework/Shader/PostProcessShader/KdPostProcessShader.h>

using SubscriptionId = uint64_t;
class Event;

class GlobalEventBus
{
public:
	static GlobalEventBus& Instance()
	{
		static GlobalEventBus instance;
		return instance;
	}

private:
	bool m_destroyed = false;  // 追加
	GlobalEventBus() : nextId(1) {}; // IDは1から開始
	~GlobalEventBus() {
		m_destroyed = true;    // 破棄前にフラグを立てる
		subscribers.clear();
	}

	using HandlerFunction = std::function<void(const Event&)>;

	// IDと関数をペアにして保持する
	using IdHandlerPair = std::pair<SubscriptionId, HandlerFunction>;
	std::unordered_map<std::type_index, std::vector<IdHandlerPair>> subscribers;

	// 一意のIDを発行するためのカウンタ
	std::atomic<SubscriptionId> nextId;

public:
	// 戻り値として SubscriptionId を返すようにする
	template <typename T>
	SubscriptionId subscribe(std::function<void(const T&)> callback) {
		auto wrapper = [callback](const Event& e) {
			callback(static_cast<const T&>(e));
			};

		SubscriptionId id = nextId++;

		// IDとラッパー関数のペアを保存
		subscribers[std::type_index(typeid(T))].push_back({ id, wrapper });

		return id; // 解除に必要なIDを呼び出し元に返す
	}

	bool IsDestroyed() const { return m_destroyed; }

	void unsubscribe(SubscriptionId id) {
		if (id == 0 || m_destroyed) return;  // フラグチェック追加
		// すべてのイベント型のリストから、指定されたIDを持つものを探して削除
		for (auto& [type, handlerList] : subscribers) {
			for (auto it = handlerList.begin(); it != handlerList.end(); ++it) {
				if (it->first == id) {
					handlerList.erase(it);
					return; // IDは一意なので見つかったら終了
				}
			}
		}
	}

	template <typename T>
	void publish(const T& event) {
		auto it = subscribers.find(std::type_index(typeid(T)));
		if (it == subscribers.end()) return;

		// コピーしてからイテレート
		auto handlerList = it->second;

		for (auto& pair : handlerList) {
			// 実行前に購読がまだ有効か確認
			auto& current = subscribers[std::type_index(typeid(T))];
			bool stillValid = std::any_of(current.begin(), current.end(),
				[&](const IdHandlerPair& p) { return p.first == pair.first; });

			if (stillValid) {
				pair.second(event);
			}
		}
	}

	void Release() {
		subscribers.clear();
	}
};

#define GLOBALEVENT GlobalEventBus::Instance()

class ScopedSubscriber {
private:
	SubscriptionId id = 0;

public:
	ScopedSubscriber() = default;
	ScopedSubscriber(SubscriptionId id) : id(id) {}

	// コピーは禁止（二重解除を防ぐため）
	ScopedSubscriber(const ScopedSubscriber&) = delete;
	ScopedSubscriber& operator=(const ScopedSubscriber&) = delete;

	// ムーブは許可
	ScopedSubscriber(ScopedSubscriber&& other) noexcept : id(other.id) { other.id = 0; }
	ScopedSubscriber& operator=(ScopedSubscriber&& other) noexcept {
		if (this != &other) {
			reset();
			id = other.id;
			other.id = 0;
		}
		return *this;
	}

	// デストラクタで自動解除！
	~ScopedSubscriber() { reset(); }

	void reset() {
		if (id != 0) {
			GLOBALEVENT.unsubscribe(id);
			id = 0;
		}
	}
};

