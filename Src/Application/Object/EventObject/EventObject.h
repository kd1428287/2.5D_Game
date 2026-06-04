#pragma once

class EventObject : public KdGameObject
{
public:
	EventObject() {};
	~EventObject()override {};

	void Init()override;
	void Update(float dt)override;
	void DrawUnLit()override;
};