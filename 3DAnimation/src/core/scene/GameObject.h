#pragma once
#include <vector>
#include "core/utils/Transform.h"
class Component;
class Renderer;

class GameObject
{
protected:
	// Default components
	Transform ObjectTransform;
	Renderer* meshRender;
	std::vector<Component*> Components;
public:
	GameObject();
	~GameObject();
	void AddComponent(Component* comp);


};