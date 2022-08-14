#include "Enemy.h"
#include <cassert>

void (Enemy::*Enemy::spFuncTable[])() = {
	&Enemy::ApporoachMove,
	&Enemy::LeaveMove
};

void Enemy::Initialize(UINT textureNumber, GeometryObject3D *object, Vector3 pos, Vector3 rot)
{
	assert(object);

	this->textureNumber = textureNumber;
	this->enemyObject = object;
	this->position = pos;
	this->rotation = rot;

	this->enemyObject->Initialize();
	this->enemyObject->SetTexNumber(this->textureNumber);
	this->enemyObject->SetPosition(this->position);
	this->enemyObject->SetRotation(this->rotation);
}

void Enemy::Update()
{
	//ˆÚ“®
	(this->*spFuncTable[static_cast<size_t>(phase)])();


	enemyObject->SetPosition(position);
	enemyObject->Update();
}

void Enemy::Draw(ID3D12GraphicsCommandList *commandList)
{
	enemyObject->Draw(commandList);
}

void Enemy::ApporoachMove()
{
	Vector3 move = {0.f,0.f,-1.f};
	position += move.normalize() * approachVelocity;

	if(position.z < 10.f)
	{
		phase = Phase::Leave;
	}
}

void Enemy::LeaveMove()
{
	Vector3 move = {-0.5f,0.5f,-1.f};
	position += move.normalize() * approachVelocity;	
}
