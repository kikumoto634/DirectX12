﻿#include "DirectXGame.h"
#include "Sprite.h"


void DirectXGame::Initialize()
{
	dxCommon = DirectXCommon::GetInstance();
	//debugText = DebugText::GetInstance();

#pragma region シーンの初期化


#pragma endregion

}

void DirectXGame::Update()
{
#pragma region シーン更新

	//debugText->Print("aaa", 100, 100);

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

	//debug
	//debugText->DrawAll(commandList);

	// スプライト描画後処理
	Sprite::PostDraw();
#pragma endregion
}

void DirectXGame::Finalize()
{
	delete sprite;
}
