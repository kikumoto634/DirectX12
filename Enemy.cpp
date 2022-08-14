#include "Enemy.h"
#include <cassert>


Enemy::~Enemy()
{
	delete state;
}

void Enemy::Initialize(UINT textureNumber, GeometryObject3D *object, Vector3 pos, Vector3 rot)
{
	assert(object);

	state = new EnemyApporoach();

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
	state->Update(this);

	enemyObject->SetPosition(position);
	enemyObject->Update();
}

void Enemy::Draw(ID3D12GraphicsCommandList *commandList)
{
	enemyObject->Draw(commandList);
}

void Enemy::ChangeState(EnemyState* newState)
{
	delete state;
	state = newState;
}



void EnemyApporoach::Update(Enemy* pEnemy)
{
	Vector3 move = {0.f,0.f,-1.f};
	pEnemy->PositionIncrement(move.normalize() * pEnemy->approachVelocity);

	if(pEnemy->GetPosition().z < 10.f)
	{
		pEnemy->ChangeState(new EnemyLeave);
	}
}

void EnemyLeave::Update(Enemy* pEnemy)
{
	Vector3 move = {-0.5f,0.5f,-1.f};
	pEnemy->PositionIncrement(move.normalize() * pEnemy->leaveVelocity);
}
