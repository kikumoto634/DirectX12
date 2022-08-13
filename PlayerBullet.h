#pragma once
#include "Vector3.h"
#include "GeometryObject3D.h"

class PlayerBullet
{
public:
	void Initialize(UINT textureNumber, GeometryObject3D* object, const Vector3& pos, const Vector3& rot, const Vector3& velocity);
	void Update();
	void Draw(ID3D12GraphicsCommandList* commandList);

private:
	Vector3 position;
	Vector3 rotation;
	GeometryObject3D* object;
	UINT textureNumber;

	//‘¬“x
	Vector3 velocity;
};