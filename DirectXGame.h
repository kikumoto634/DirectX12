#pragma once
#include "DirectXCommon.h"

class DirectXGame
{
public:
	void Initialize();

	void Update();

	void Draw();

	void Finalize();

private:
	DirectXCommon* dxCommon= nullptr;
};

