#include "EnemyBullet.h"
#include <cassert>
#include <cmath>

void EnemyBullet::Initialize(UINT textureNumber, GeometryObject3D *object, const Vector3 &pos, const Vector3 &velocity)
{
	assert(object);

	this->object = object;
	this->position = pos;
	this->velocity = velocity;
	this->textureNumber = textureNumber;

	this->object->Initialize();
	this->object->SetTexNumber(this->textureNumber);
	this->object->SetPosition(position);
	this->object->SetScale({0.5f, 0.5f, 3.f});
	this->object->SetColor({0,1,0,1});

	//��@1
	/*//Y������p�x(��Y)
	rotation.y = std::atan2(velocity.x, velocity.z);
	float sin = std::sin(rotation.y);
	float cos = std::cos(rotation.y);
	//Y�������-��y�񂷉�]�s��
	static Matrix4 matrix
	{
		cos,	0.0f,	-sin,	0.0f,
		0.0f,	1.0f,	0.0f,	0.0f,
		sin,	0.0f,	cos,	0.0f,
		0.0f,	0.0f,	0.0f,	1.0f
	};
	//velocity�ɉ�]�s����|���AvelocityZ�����߂�
	Vector3 velocityZ = velocity*matrix;
	//X������p�x(��X)
	rotation.x = std::atan2(-velocityZ.y, velocityZ.z);*/

	//��@2
	//Y������p�x(��Y)
	rotation.y = std::atan2(velocity.x, velocity.z);
	//���������̒��������߂�
	float temp = sqrt(velocity.x*velocity.x + velocity.z*velocity.z);
	//X������p�x(��X)
	rotation.x = std::atan2(-velocity.y, temp);

	this->object->SetRotation(rotation);
}

void EnemyBullet::Update()
{
	if(--deathTimer <= 0)
	{
		isDead = true;
	}

	position += velocity;
	object->SetPosition(position);

	object->Update();
}

void EnemyBullet::Draw(ID3D12GraphicsCommandList *commandList)
{
	object->Draw(commandList);
}
