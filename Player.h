#pragma once
#include <memory>
#include "GeometryObject3D.h"
#include "Vector3.h"
#include "Input.h"

class Player
{
public:
	void Initialize(Input* input, UINT textureNumber, Vector3 pos = {0,0,0}, Vector3 rot = {0,0,0});
	void Update();
	void Draw(ID3D12GraphicsCommandList* commandList);

private:
	std::unique_ptr<GeometryObject3D> object;

	Vector3 position;
	Vector3 rotation;
	UINT textureNumber;

	Input* input = nullptr;

	float velocity = 3.f;
};

