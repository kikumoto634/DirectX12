#include "DirectXGame.h"
#include "Sprite.h"


void DirectXGame::Initialize()
{
	dxCommon = DirectXCommon::GetInstance();

#pragma region シーンの初期化

	textureHandle = TextureManager::Load("Morley_Idel.png");
	sprite = Sprite::Create(textureHandle, {100, 50});

#pragma endregion

}

void DirectXGame::Update()
{
#pragma region シーン更新

#pragma endregion
}

void DirectXGame::Draw()
{
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();

#pragma region 背景スプライト描画
	// 背景スプライト描画前処理
	Sprite::PreDraw(commandList);

	//BACK

	// スプライト描画後処理
	Sprite::PostDraw();
	// 深度バッファクリア
	dxCommon->ClearDepthBuffer();
#pragma endregion

#pragma region 3Dオブジェクト描画
	// 3Dオブジェクト描画前処理


	// 3Dオブジェクト描画後処理

#pragma endregion

#pragma region 前景スプライト描画
	// 背景スプライト描画前処理
	Sprite::PreDraw(commandList);

	//UI
	sprite->Draw();

	// スプライト描画後処理
	Sprite::PostDraw();
#pragma endregion
}

void DirectXGame::Finalize()
{
	delete sprite;
}
