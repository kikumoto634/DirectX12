#include "Player.h"

using namespace std;

void Player::Initialize(Input* input, UINT textureNumber, Vector3 pos, Vector3 rot)
{
	this->input = input;

	object->Initialize();
	object->SetTexNumber(textureNumber);
	object->SetPosition(pos);
	object->SetRotation(rot);
}

void Player::Update()
{
	Vector3 move = {0, 0, 0};

	if(input->Push(DIK_D))
	{
		move = {1,0,0};
	}
	else if(input->Push(DIK_A))
	{
		move = {-1,0,0};
	}

	position += move * velocity;

	object->SetPosition(position);
	object->Update();
}

void Player::Draw(ID3D12GraphicsCommandList* commandList)
{
	object->Draw(commandList);
}
