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

	//YŽ²Žü‚èŠp“x(ƒÆY)
	rotation.y = std::atan2(velocity.x, velocity.z);
	//‰¡Ž²•ûŒü‚Ì’·‚³‚ð‹‚ß‚é
	float temp = sqrt(velocity.x*velocity.x + velocity.z*velocity.z);
	//XŽ²Žü‚èŠp“x(ƒÆX)
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

void EnemyBullet::OnCollision()
{
	isDead = true;
}
