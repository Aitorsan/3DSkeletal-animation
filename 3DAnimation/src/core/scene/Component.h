#pragma once
class GameObject;

class Component
{
	GameObject* Parent;
public:
	Component(GameObject* parent): Parent{parent}{}
	virtual void SetUp(){}
	virtual void Update() {}
	virtual void Input() {}
	virtual void Render(){}

	GameObject* GetParent() { return Parent; }
};