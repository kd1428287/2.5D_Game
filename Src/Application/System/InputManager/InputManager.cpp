#include "InputManager.h"
#include "../../main.h"

void InputManager::Init()
{
	for (int i = 0; i < 256; ++i) {
		currentKeys[i] = false;
		prevKeys[i] = false;
	}
}

void InputManager::Update()
{}

Math::Vector2 InputManager::GetMousePos()
{
	POINT mp{};
	GetCursorPos(&mp);
	ScreenToClient(Application::Instance().GetWindowHandle(), &mp);

	mp.x -= 1280 / 2;
	mp.y = 720 / 2 - mp.y;
	Math::Vector2 answer;
	answer = { (float)mp.x,(float)mp.y };
	return answer;
}
