#pragma once
#include <memory>
#include <list>

#include "GeometryObject3D.h"
#include "Vector3.h"
#include "EnemyBullet.h"

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
	static const int kFireInterval = 60;

public:
	const float approachVelocity = 2.f;
	const float leaveVelocity = 4.f;

public:
	Enemy() = default;
	~Enemy();

	void Initialize(UINT textureNumber, GeometryObject3D* object, Vector3 pos = {0,0,50}, Vector3 rot = {0,XMConvertToRadians(180.f),0});
	void Update();
	void Draw(ID3D12GraphicsCommandList* commandList);

	//����
	void Fire();
	void FireTimerDecrement(int frame)	{fireTimer -= frame;}
	void FireTimerInitialize()	{fireTimer = kFireInterval;}

	//��ԑJ��
	void ChangeState(EnemyState* newState);

	//�ڋ߃t�F�[�Y������
	void ApporoachInitialize();

	//���Z
	void PositionIncrement(Vector3 pos)	{position += pos;}

	//Get
	Vector3 GetPosition()	{return position;}
	int32_t GetFireTimer()	{return fireTimer;}

private:
	//�I�u�W�F�N�g
	GeometryObject3D* enemyObject = nullptr;

	std::list<std::unique_ptr<EnemyBullet>> bullets;
	std::list<std::unique_ptr<GeometryObject3D>> bulletsObject;
	//���˃^�C�}�[
	int32_t fireTimer = 0;

	//���
	Vector3 position;
	Vector3 rotation;
	UINT textureNumber;

	//���
	EnemyState* state = nullptr;
};