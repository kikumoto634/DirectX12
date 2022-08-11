#pragma once
#include <memory>
#include "GeometryObject3D.h"
#include "Vector3.h"
#include "Input.h"

class Player
{
public:
	void Initialize(Input* input, UINT textureNumber, GeometryObject3D* object, Vector3 pos = {0,0,50}, Vector3 rot = {0,0,0});
	void Update();
	void Draw(ID3D12GraphicsCommandList* commandList);

private:
	Vector3 MovementInput();
	Vector3 RotationInput();

private:
	GeometryObject3D* object;
	Input* input = nullptr;

	Vector3 position;
	Vector3 rotation;
	UINT textureNumber;

	float velocity_Movement = 1.f;
	float velocity_Rotation = 0.025f;
};

