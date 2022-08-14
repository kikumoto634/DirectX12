#pragma once
#include "GeometryObject3D.h"
#include "Vector3.h"

class Enemy;

class EnemyState
{
public:
	virtual void Update(Enemy* pEnemy) = 0;
};

//接近
class EnemyApporoach : public EnemyState
{
public:
	void Update(Enemy* pEnemy);
};

//離脱
class EnemyLeave : public EnemyState
{
public:
	void Update(Enemy* pEnemy);
};

/// <summary>
/// 敵
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

	//状態遷移
	void ChangeState(EnemyState* newState);

	//加算
	void PositionIncrement(Vector3 pos)	{position += pos;}

	//Get
	Vector3 GetPosition()	{return position;}

private:
	//オブジェクト
	GeometryObject3D* enemyObject = nullptr;

	//情報
	Vector3 position;
	Vector3 rotation;
	UINT textureNumber;

	//状態
	EnemyState* state = nullptr;
};