#pragma once
#include "Vector3.h"
#include "GeometryObject3D.h"

class PlayerBullet
{
public:
	void Initialize(UINT textureNumber, GeometryObject3D* object, const Vector3& pos, const Vector3& rot, const Vector3& velocity);
	void Update();
	void Draw(ID3D12GraphicsCommandList* commandList);

	//衝突判定
	void OnCollision();

	bool GetIsDead() const {return isDead;}
	Vector3 GetPosition()	{return position;}

private:
	static const int32_t kLifeTime = 60 * 5;

private:
	Vector3 position;
	Vector3 rotation;
	GeometryObject3D* object;
	UINT textureNumber;

	//速度
	Vector3 velocity;

	//デスタイマー
	int32_t deathTimer = kLifeTime;
	//デスフラグ
	bool isDead = false;
};