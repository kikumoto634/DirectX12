#pragma once
#include "Vector3.h"
#include "GeometryObject3D.h"
#include "Collider.h"

class PlayerBullet : public Collider
{
public:
	void Initialize(UINT textureNumber, GeometryObject3D* object, const Vector3& pos, const Vector3& rot, const Vector3& velocity);
	void Update();
	void Draw(ID3D12GraphicsCommandList* commandList);

	//�Փ˔���
	void OnCollision() override;

	bool GetIsDead() const {return isDead;}
	const Vector3 GetPosition() override	{return position;}

private:
	static const int32_t kLifeTime = 60 * 5;

private:
	Vector3 position;
	Vector3 rotation;
	GeometryObject3D* object;
	UINT textureNumber;

	//���x
	Vector3 velocity;

	//�f�X�^�C�}�[
	int32_t deathTimer = kLifeTime;
	//�f�X�t���O
	bool isDead = false;
};