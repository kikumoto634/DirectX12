#pragma once
#include "Vector3.h"
#include "CollisionConfig.h"

class Collider
{
public:
	//衝突時
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
	//衝突半径
	float radius = 5.f;//デフォルト値

	//衝突属性(自陣)
	uint32_t collisionAttribute = 0xffffffff;
	//衝突マスク(敵)
	uint32_t collisionMask = 0xffffffff;
};

