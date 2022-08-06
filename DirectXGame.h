#pragma once
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "Sprite.h"
//#include "DebugText.h"

//#include "Model.h"
//#include "Object3D.h"
//#include "FbxLoader.h"

#include "WorldTransform.h"
#include "ViewProjection.h"

#include <memory>

class DirectXGame
{
public:
	void Initialize();

	void Update();

	void Draw();

	void Finalize();

private:
	DirectXCommon* dxCommon= nullptr;
	//DebugText* debugText = nullptr;

	uint32_t textureHandle = 0;
	Sprite* sprite = nullptr;

	//std::unique_ptr<Model> model;
	//std::unique_ptr<Object3D> modelObject;

	WorldTransform worldTransform;
	ViewProjection viewProjection;
};

