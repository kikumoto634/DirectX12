#pragma once
#include <list>

#include "Collider.h"

class CollisionManager
{
public:
	
	//Set
	//���X�g�ǉ�
	void SetCollision(Collider* collider);

	//���X�g�̃N���A
	void CollisionClear();

public:
	//�R���C�_�[���X�g
	std::list<Collider*> colliders;
};

