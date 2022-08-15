#include "Enemy.h"
#include <cassert>

using namespace std;

Enemy::~Enemy()
{
	delete state;
}

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

	state = new EnemyApporoach();
	ApporoachInitialize();

	//�Փˑ����ݒ�
	SetCollisionAttribute(kCollisionAttributeEnemy);
	//�ՓˑΏۂ������̑����ȊO�ɐݒ�
	SetCollisionMask(kCollisionAttributeEnemy);
}

void Enemy::Update()
{
	//�f�X�t���O�̂������e�̍폜
	bullets.remove_if([](unique_ptr<EnemyBullet>& bullet)
	{
		return bullet->GetIsDead();
	});
	//�I�������^�C�}�[�폜
	timedCalls.remove_if([](unique_ptr<TimedCall>& time)
	{
		return time->IsFinished();
	});

	state->Update(this);

	enemyObject->SetPosition(position);
	enemyObject->Update();

	//�S�v�f����
	for(unique_ptr<EnemyBullet>& bullet : bullets)
	{
		bullet->Update();
	}
	for(unique_ptr<TimedCall>& timer : timedCalls)
	{
		timer->Update();
	}
}

void Enemy::Draw(ID3D12GraphicsCommandList *commandList)
{
	enemyObject->Draw(commandList);

	for(unique_ptr<EnemyBullet>& bullet : bullets)
	{
		bullet->Draw(commandList);
	}
}

void Enemy::Fire()
{
	assert(player);

	//���x
	const float bulletSpeed = 5.0f;

	//�U���e
	Vector3 playerPos = player->GetPosition();
	Vector3 enemyPos = GetPosition();
	enemyPos = playerPos - enemyPos;
	enemyPos = enemyPos.normalize();

	//����������
	unique_ptr<EnemyBullet> newBullet = make_unique<EnemyBullet>();
	unique_ptr<GeometryObject3D> newBulletObject = make_unique<GeometryObject3D>();

	newBullet->Initialize(textureNumber, newBulletObject.get(), position, enemyPos*bulletSpeed);
	
	bullets.push_back(move(newBullet));
	bulletsObject.push_back(move(newBulletObject));
}

void Enemy::FireTimeReset()
{
	//���˃^�C�}�[���Z�b�g
	timedCalls.push_back(std::make_unique<TimedCall>(std::bind(&Enemy::FireTimeReset, this), std::bind(&Enemy::Fire, this), kFireInterval));
}

void Enemy::ChangeState(EnemyState* newState)
{
	delete state;
	state = newState;
}

void Enemy::ApporoachInitialize()
{
	//���˃^�C�}�[���Z�b�g����
	FireTimeReset();
}

void Enemy::ApporoachFinalize()
{
	timedCalls.clear();
}

void Enemy::OnCollision()
{
}

void EnemyApporoach::Update(Enemy* pEnemy)
{
	Vector3 move = {0.f,0.f,-1.f};
	pEnemy->PositionIncrement(move.normalize() * pEnemy->approachVelocity);

	if(pEnemy->GetPosition().z < 50.f)
	{
		pEnemy->ApporoachFinalize();
		pEnemy->ChangeState(new EnemyLeave);
	}
}

void EnemyLeave::Update(Enemy* pEnemy)
{
	Vector3 move = {-0.5f,0.5f,-1.f};
	pEnemy->PositionIncrement(move.normalize() * pEnemy->leaveVelocity);
}
