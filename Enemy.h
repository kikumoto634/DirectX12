#pragma once
#include "GeometryObject3D.h"
#include "Vector3.h"

class Enemy;

class EnemyState
{
public:
	virtual void Update(Enemy* pEnemy) = 0;
};

//�ڋ�
class EnemyApporoach : public EnemyState
{
public:
	void Update(Enemy* pEnemy);
};

//���E
class EnemyLeave : public EnemyState
{
public:
	void Update(Enemy* pEnemy);
};

/// <summary>
/// �G
/// </summary> 
class Enemy
{
public:
	const float approachVelocity = 2.f;
	const float leaveVelocity = 4.f;

public:
	Enemy() = default;
	~Enemy();

	void Initialize(UINT textureNumber, GeometryObject3D* object, Vector3 pos = {0,0,50}, Vector3 rot = {0,0,0});
	void Update();
	void Draw(ID3D12GraphicsCommandList* commandList);

	//��ԑJ��
	void ChangeState(EnemyState* newState);

	//���Z
	void PositionIncrement(Vector3 pos)	{position += pos;}

	//Get
	Vector3 GetPosition()	{return position;}

private:
	//�I�u�W�F�N�g
	GeometryObject3D* enemyObject = nullptr;

	//���
	Vector3 position;
	Vector3 rotation;
	UINT textureNumber;

	//���
	EnemyState* state = nullptr;
};