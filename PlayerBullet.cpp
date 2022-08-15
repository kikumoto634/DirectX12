#include "PlayerBullet.h"
#include <cassert>

void PlayerBullet::Initialize(UINT textureNumber, GeometryObject3D *object, const Vector3 &pos, const Vector3& rot, const Vector3& velocity)
{
	assert(object);

	this->object = object;
	this->position = pos;
	this->rotation = rot;
	this->velocity = velocity;
	this->textureNumber = textureNumber;

	this->object->Initialize();
	this->object->SetTexNumber(this->textureNumber);
	this->object->SetPosition(position);
	this->object->SetRotation(rotation);
	this->object->SetColor({1,0,0,1});

	//衝突属性設定
	SetCollisionAttribute(kCollisionAttributePlayer);
	//衝突対象を自分の属性以外に設定
	SetCollisionMask(kCollisionAttributePlayer);
}

void PlayerBullet::Update()
{
	if(--deathTimer <= 0)
	{
		isDead = true;
	}

	position += velocity;
	object->SetPosition(position);

	object->Update();
}

void PlayerBullet::Draw(ID3D12GraphicsCommandList *commandList)
{
	object->Draw(commandList);
}

void PlayerBullet::OnCollision()
{
	isDead = true;
}
