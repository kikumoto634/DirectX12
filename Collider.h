#pragma once
#include "Vector3.h"

class Collider
{
public:

	//�Փˎ�
	virtual void OnCollision() = 0;

	//Get
	const float GetRadius()	{return radius;}
	virtual const Vector3 GetPosition() = 0;

	//Set
	float SetRadius(const float radius)	{this->radius = radius;}

private:
	//�Փ˔��a
	float radius = 5.f;//�f�t�H���g�l
};

