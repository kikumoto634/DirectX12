#pragma once
#include "Vector3.h"
#include "CollisionConfig.h"

class Collider
{
public:
	//�Փˎ�
	virtual void OnCollision() = 0;

	//Get
	const float GetRadius()	{return radius;}
	virtual const Vector3 GetPosition() = 0;

	const uint32_t GetCollisionAttribute()	{return collisionAttribute;}
	const uint32_t GetCollisionMask()	{return collisionMask;}

	//Set
	float SetRadius(const float radius)	{this->radius = radius;}

	void SetCollisionAttribute(const uint32_t bute)	{this->collisionAttribute = bute;}
	void SetCollisionMask(const uint32_t bute)	{this->collisionMask = bute;}

private:
	//�Փ˔��a
	float radius = 5.f;//�f�t�H���g�l

	//�Փˑ���(���w)
	uint32_t collisionAttribute = 0xffffffff;
	//�Փ˃}�X�N(�G)
	uint32_t collisionMask = 0xffffffff;
};

