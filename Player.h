#pragma once
#include <memory>
#include <list>

#include "GeometryObject3D.h"
#include "Vector3.h"
#include "Input.h"
#include "PlayerBullet.h"
#include "Collider.h"

class Player : public Collider
{
public:
	void Initialize(Input* input, UINT textureNumber, GeometryObject3D* object, Vector3 pos = {0,0,0}, Vector3 rot = {0,0,0});
	void Update();
	void Draw(ID3D12GraphicsCommandList* commandList);

	//�Փ˔���
	void OnCollision() override;

	//Get
	const Vector3 GetPosition() override	{return position;}
	//�e���X�g�擾
	const std::list<std::unique_ptr<PlayerBullet>>& GetBullets()	{return bullets;}

private:
	Vector3 MovementInput();
	Vector3 RotationInput();

	void Attack();

private:
	//�I�u�W�F�N�g
	GeometryObject3D* playerObject = nullptr;

	std::list<std::unique_ptr<PlayerBullet>> bullets;
	std::list<std::unique_ptr<GeometryObject3D>> bulletsObject;
	Input* input = nullptr;

	//���
	Vector3 position;
	Vector3 rotation;
	UINT textureNumber;

	//�ϐ�
	float velocity_Movement = 1.f;
	float velocity_Rotation = 0.025f;
};

