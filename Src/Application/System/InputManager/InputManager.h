#pragma once

class InputManager
{
public:
	static InputManager& Instance()
	{
		static InputManager instance;
		return instance;
	}

	void Init();
	void Update();
	
	Math::Vector2 GetMousePos();
	//Math::Vector2 GetMouseDir();

	// キーが「押された瞬間」 (Trigger)
	bool IsTriggered(int key) const {
		return currentKeys[key] && !prevKeys[key];
	}

	// キーが「押され続けている」 (Press)
	bool IsPressed(int key) const {
		return currentKeys[key];
	}

	// キーが「離された瞬間」 (Release)
	bool IsReleased(int key) const {
		return !currentKeys[key] && prevKeys[key];
	}

	// キーが押されているか-いないならfalse
	bool IsEntered()const
	{
		bool check = false;

		for (int i = 0; i < 256; ++i) {
			if (currentKeys[i] == true)
			{
				check = true;
				break;
			}
		}

		return check;
	}

private:

	InputManager() {};
	~InputManager() {};

	std::array<bool, 256> currentKeys;
	std::array<bool, 256> prevKeys;

};