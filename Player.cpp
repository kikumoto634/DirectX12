#include "Player.h"
#include <cassert>

using namespace std;

void Player::Initialize(Input* input, UINT textureNumber, GeometryObject3D* object, Vector3 pos, Vector3 rot)
{
	assert(input);
	assert(object);

	this->input = input;
	this->playerObject = object;
	position = pos;
	rotation = rot;
	this->textureNumber = textureNumber;

	this->playerObject->Initialize();
	this->playerObject->SetTexNumber(this->textureNumber);
	this->playerObject->SetPosition(position);
	this->playerObject->SetRotation(rotation);
}

void Player::Update()
{
	//�f�X�t���O�̂������e�̍폜
	bullets.remove_if([](unique_ptr<PlayerBullet>& bullet)
	{
			return bullet->GetIsDead();
	});

	//�ړ�
	position += MovementInput().normalize() * velocity_Movement;
	//��]
	rotation += RotationInput().normalize() * velocity_Rotation;

	//����
	const float moveLimitX = 100.f;
	const float moveLimitY = 50.f;
	position.x = max(position.x, -moveLimitX);
	position.x = min(position.x, +moveLimitX);
	position.y = max(position.y, -moveLimitY);
	position.y = min(position.y, +moveLimitY);

	//�X�V
	playerObject->SetPosition(position);
	playerObject->SetRotation(rotation);
	playerObject->Update();

	Attack();
	for(unique_ptr<PlayerBullet>& bullet : bullets)
	{
		bullet->Update();
	}
}

void Player::Draw(ID3D12GraphicsCommandList* commandList)
{
	playerObject->Draw(commandList);

	for(unique_ptr<PlayerBullet>& bullet : bullets)
	{
		bullet->Draw(commandList);
	}
}

Vector3 Player::MovementInput()
{
	Vector3 move = {0, 0, 0};

	if(input->Push(DIK_RIGHT))
	{
		move.x += 1.f;
	}
	else if(input->Push(DIK_LEFT))
	{
		move.x -= 1.0f;
	}

	if(input->Push(DIK_UP))
	{
		move.y += 1.0f;
	}
	else if(input->Push(DIK_DOWN))
	{
		move.y -= 1.0f;
	}

	return move;
}

Vector3 Player::RotationInput()
{
	Vector3 rot = {0,0,0};
	if(input->Push(DIK_A))
	{
		rot.y -= 1.f;
	}
	else if(input->Push(DIK_D))
	{
		rot.y += 1.f;
	}

	if(input->Push(DIK_W))
	{
		rot.x += 1.f;
	}
	else if(input->Push(DIK_S))
	{
		rot.x -= 1.f;
	}

	return rot;
}

void Player::Attack()
{
	if(input->Trigger(DIK_SPACE))
	{
		//���x
		const float bulletSpeed = 1.0f;
		Vector3 velocity(0, 0, bulletSpeed);

		//���x�Ǝ��@�̉�]
		velocity.x = velocity.x*playerObject->GetWorld().r[0].m128_f32[0] + velocity.y*playerObject->GetWorld().r[1].m128_f32[0] + velocity.z*playerObject->GetWorld().r[2].m128_f32[0];
		velocity.y = velocity.x*playerObject->GetWorld().r[0].m128_f32[1] + velocity.y*playerObject->GetWorld().r[1].m128_f32[1] + velocity.z*playerObject->GetWorld().r[2].m128_f32[1];
		velocity.z = velocity.x*playerObject->GetWorld().r[0].m128_f32[2] + velocity.y*playerObject->GetWorld().r[1].m128_f32[2] + velocity.z*playerObject->GetWorld().r[2].m128_f32[2];

		//����������
		unique_ptr<PlayerBullet> newBullet = make_unique<PlayerBullet>();
		unique_ptr<GeometryObject3D> newBulletObject = make_unique<GeometryObject3D>();

		newBullet->Initialize(textureNumber, newBulletObject.get(), position, rotation, velocity);

		bullets.push_back(move(newBullet));
		bulletsObject.push_back(move(newBulletObject));
	}
}
