#pragma once
#include <typeindex>

class Event;

class LocalEventBus
{
public:
	static LocalEventBus& Instance()
	{
		static LocalEventBus instance;
		return instance;
	}
private:

	LocalEventBus() {};
	~LocalEventBus() {};

	// 基底クラス(Event)の参照を受け取る汎用関数の型
	using HandlerFunction = std::function<void(const Event&)>;

	// イベントの型(type_index)をキーにして、コールバックのリストを保持する辞書
	std::unordered_map<std::type_index, std::vector<HandlerFunction>> subscribers;

public:
	// 購読 (Subscribe)
	// 特定のイベント型 T のみを処理する関数を受け取る
	template <typename T>
	void subscribe(std::function<void(const T&)> callback) {
		// T型専用のコールバックを、基底クラス(Event)を受け取るラムダ式でラップする（型の消去）
		auto wrapper = [callback](const Event& e) {
			// 安全にダウンキャストして元の関数を実行
			callback(static_cast<const T&>(e));
			};

		// 型情報(typeid)をキーにしてリストに追加
		subscribers[std::type_index(typeid(T))].push_back(wrapper);
	}

	// 発行 (Publish)
	// 任意のイベント T を受け取り、登録されているリスナーに配る
	template <typename T>
	void publish(const T& event) {
		auto it = subscribers.find(std::type_index(typeid(T)));
		if (it != subscribers.end()) {
			for (auto& handler : it->second) {
				handler(event);
			}
		}
	}
};