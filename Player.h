#pragma once
#include <memory>
#include <list>

#include "GeometryObject3D.h"
#include "Vector3.h"
#include "Input.h"
#include "PlayerBullet.h"

class Player
{
public:
	void Initialize(Input* input, UINT textureNumber, GeometryObject3D* object, Vector3 pos = {0,0,50}, Vector3 rot = {0,0,0});
	void Update();
	void Draw(ID3D12GraphicsCommandList* commandList);

private:
	Vector3 MovementInput();
	Vector3 RotationInput();

	void Attack();

private:
	//オブジェクト
	GeometryObject3D* playerObject = nullptr;

	std::list<std::unique_ptr<PlayerBullet>> bullets;
	std::list<std::unique_ptr<GeometryObject3D>> bulletsObject;
	Input* input = nullptr;

	//情報
	Vector3 position;
	Vector3 rotation;
	UINT textureNumber;

	//変数
	float velocity_Movement = 1.f;
	float velocity_Rotation = 0.025f;
};

