#include "Player.h"
#include <cassert>

using namespace std;

void Player::Initialize(Input* input, UINT textureNumber, GeometryObject3D* object, Vector3 pos, Vector3 rot)
{
	assert(input);
	assert(object);

	this->input = input;
	this->object = object;
	position = pos;
	rotation = rot;
	this->textureNumber = textureNumber;

	this->object->Initialize();
	this->object->SetTexNumber(this->textureNumber);
	this->object->SetPosition(position);
	this->object->SetRotation(rotation);
}

void Player::Update()
{
	//ˆÚ“®
	position += MovementInput().normalize() * velocity_Movement;
	//‰ñ“]
	rotation += RotationInput().normalize() * velocity_Rotation;

	//§ŒÀ
	const float moveLimitX = 100.f;
	const float moveLimitY = 50.f;
	position.x = max(position.x, -moveLimitX);
	position.x = min(position.x, +moveLimitX);
	position.y = max(position.y, -moveLimitY);
	position.y = min(position.y, +moveLimitY);

	//XV
	object->SetPosition(position);
	object->SetRotation(rotation);
	object->Update();
}

void Player::Draw(ID3D12GraphicsCommandList* commandList)
{
	object->Draw(commandList);
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

	return rot;
}
