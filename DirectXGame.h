#pragma once
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "Sprite.h"
//#include "DebugText.h"

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
};

