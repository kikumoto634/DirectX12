#pragma once
#include <list>

#include "Collider.h"

class CollisionManager
{
public:
	
	//Set
	//リスト追加
	void SetCollision(Collider* collider);

	//リストのクリア
	void CollisionClear();

public:
	//コライダーリスト
	std::list<Collider*> colliders;
};

