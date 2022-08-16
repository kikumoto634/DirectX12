#pragma once
#include "Model.h"
#include "Object3D.h"
#include "Vector3.h"

#include <memory>

/// <summary>
/// “V‹…
/// </summary>
class Skydome
{
public:
	void Initialize(Model* model, Object3D* modelObject, Vector3 pos = {0,0,0});
	void Update();
	void Draw(ID3D12GraphicsCommandList* commandList);

private:
	Model* model = nullptr;
	Object3D* modelObject;

	Vector3 position;
};

