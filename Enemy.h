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
	const float leaveVelocity = 3.f;

public:
	Enemy() = default;
	~Enemy();

	void Initialize(UINT textureNumber, GeometryObject3D* object, Vector3 pos = {0,0,50}, Vector3 rot = {0,XMConvertToRadians(180.f),0});
	void Update();
	void Draw(ID3D12GraphicsCommandList* commandList);

	//�e�̔��ˁA�^�C�}�[���Z�b�g����R�[���o�b�N
	void FireTimeReset();

	//��ԑJ��
	void ChangeState(EnemyState* newState);

	//�ڋ߃t�F�[�Y������
	void ApporoachInitialize();
	//�ڋ߃t�F�[�Y�㏈��
	void ApporoachFinalize();

	//���Z
	void PositionIncrement(Vector3 pos)	{position += pos;}

	//Get
	Vector3 GetPosition()	{return position;}

private:
	//����
	void Fire();

private:
	//�I�u�W�F�N�g
	GeometryObject3D* enemyObject = nullptr;

	std::list<std::unique_ptr<EnemyBullet>> bullets;
	std::list<std::unique_ptr<GeometryObject3D>> bulletsObject;
	//���������̃��X�g
	std::list<std::unique_ptr<TimedCall>> timedCalls;

	//���
	Vector3 position;
	Vector3 rotation;
	UINT textureNumber;

	//���
	EnemyState* state = nullptr;
};