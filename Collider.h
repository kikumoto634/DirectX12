#pragma once
#include "Vector3.h"

class Collider
{
public:

	//衝突時
	virtual void OnCollision() = 0;

	//Get
	const float GetRadius()	{return radius;}
	virtual const Vector3 GetPosition() = 0;

	//Set
	float SetRadius(const float radius)	{this->radius = radius;}

private:
	//衝突半径
	float radius = 5.f;//デフォルト値
};

