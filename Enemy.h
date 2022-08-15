#pragma once
#include <memory>
#include <list>

#include "GeometryObject3D.h"
#include "Vector3.h"
#include "EnemyBullet.h"
#include "TimedCall.h"

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
	static const int kFireInterval = 60;

public:
	const float approachVelocity = 2.f;
	const float leaveVelocity = 3.f;

public:
	Enemy() = default;
	~Enemy();

	void Initialize(UINT textureNumber, GeometryObject3D* object, Vector3 pos = {0,0,50}, Vector3 rot = {0,XMConvertToRadians(180.f),0});
	void Update();
	void Draw(ID3D12GraphicsCommandList* commandList);

	//弾の発射、タイマーリセットするコールバック
	void FireTimeReset();

	//状態遷移
	void ChangeState(EnemyState* newState);

	//接近フェーズ初期化
	void ApporoachInitialize();
	//接近フェーズ後処理
	void ApporoachFinalize();

	//加算
	void PositionIncrement(Vector3 pos)	{position += pos;}

	//Get
	Vector3 GetPosition()	{return position;}

private:
	//発射
	void Fire();

private:
	//オブジェクト
	GeometryObject3D* enemyObject = nullptr;

	std::list<std::unique_ptr<EnemyBullet>> bullets;
	std::list<std::unique_ptr<GeometryObject3D>> bulletsObject;
	//時限発動のリスト
	std::list<std::unique_ptr<TimedCall>> timedCalls;

	//情報
	Vector3 position;
	Vector3 rotation;
	UINT textureNumber;

	//状態
	EnemyState* state = nullptr;
};