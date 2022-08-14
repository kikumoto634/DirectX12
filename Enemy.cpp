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

	state = new EnemyApporoach();

	this->textureNumber = textureNumber;
	this->enemyObject = object;
	this->position = pos;
	this->rotation = rot;

	this->enemyObject->Initialize();
	this->enemyObject->SetTexNumber(this->textureNumber);
	this->enemyObject->SetPosition(this->position);
	this->enemyObject->SetRotation(this->rotation);

	ApporoachInitialize();
}

void Enemy::Update()
{
	//デスフラグのたった弾の削除
	bullets.remove_if([](unique_ptr<EnemyBullet>& bullet)
	{
		return bullet->GetIsDead();
	});

	state->Update(this);

	enemyObject->SetPosition(position);
	enemyObject->Update();

	for(unique_ptr<EnemyBullet>& bullet : bullets)
	{
		bullet->Update();
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
	//速度
	const float bulletSpeed = 5.0f;
	Vector3 velocity(0, 0, bulletSpeed);

	//速度と自機の回転
	velocity.x = velocity.x*enemyObject->GetWorld().r[0].m128_f32[0] + velocity.y*enemyObject->GetWorld().r[1].m128_f32[0] + velocity.z*enemyObject->GetWorld().r[2].m128_f32[0];
	velocity.y = velocity.x*enemyObject->GetWorld().r[0].m128_f32[1] + velocity.y*enemyObject->GetWorld().r[1].m128_f32[1] + velocity.z*enemyObject->GetWorld().r[2].m128_f32[1];
	velocity.z = velocity.x*enemyObject->GetWorld().r[0].m128_f32[2] + velocity.y*enemyObject->GetWorld().r[1].m128_f32[2] + velocity.z*enemyObject->GetWorld().r[2].m128_f32[2];

	//生成初期化
	unique_ptr<EnemyBullet> newBullet = make_unique<EnemyBullet>();
	unique_ptr<GeometryObject3D> newBulletObject = make_unique<GeometryObject3D>();

	newBullet->Initialize(textureNumber, newBulletObject.get(), position, rotation, velocity);
	
	bullets.push_back(move(newBullet));
	bulletsObject.push_back(move(newBulletObject));
}

void Enemy::ChangeState(EnemyState* newState)
{
	delete state;
	state = newState;
}

void Enemy::ApporoachInitialize()
{
	FireTimerInitialize();
}



void EnemyApporoach::Update(Enemy* pEnemy)
{
	//発射処理
	pEnemy->FireTimerDecrement(1);
	if(pEnemy->GetFireTimer() < 0)
	{
		pEnemy->Fire();
		pEnemy->FireTimerInitialize();
	}

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
