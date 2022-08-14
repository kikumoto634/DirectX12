#pragma once
#include "GeometryObject3D.h"
#include "Vector3.h"

/// <summary>
/// �G
/// </summary>
class Enemy
{
private:
	enum class Phase
	{
		Apporoach,
		Leave,
	};

private:
	static void (Enemy::*spFuncTable[])();

public:
	void Initialize(UINT textureNumber, GeometryObject3D* object, Vector3 pos = {0,0,50}, Vector3 rot = {0,0,0});
	void Update();
	void Draw(ID3D12GraphicsCommandList* commandList);

	void ApporoachMove();
	void LeaveMove();

private:
	//�I�u�W�F�N�g
	GeometryObject3D* enemyObject = nullptr;

	//���
	Vector3 position;
	Vector3 rotation;
	UINT textureNumber;

	float approachVelocity = 2.f;
	float leaveVelocity = 4.f;

	Phase phase = Phase::Apporoach;
};

