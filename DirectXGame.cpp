#include "DirectXGame.h"


void DirectXGame::Initialize()
{
	dxCommon = DirectXCommon::GetInstance();

#pragma region シーンの初期化



#pragma endregion

}

void DirectXGame::Update()
{
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();

#pragma region シーン更新

#pragma endregion
}

void DirectXGame::Draw()
{
#pragma region 背景スプライト描画
	// 背景スプライト描画前処理


	// スプライト描画後処理

	// 深度バッファクリア
	dxCommon->ClearDepthBuffer();
#pragma endregion

#pragma region 3Dオブジェクト描画
	// 3Dオブジェクト描画前処理


	// 3Dオブジェクト描画後処理

#pragma endregion

#pragma region 前景スプライト描画
	// 前景スプライト描画前処理


	// デバッグテキストの描画

	
	// スプライト描画後処理


#pragma endregion
}

void DirectXGame::Finalize()
{

}
