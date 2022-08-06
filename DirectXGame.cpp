#include "DirectXGame.h"
#include "Sprite.h"


void DirectXGame::Initialize()
{
	dxCommon = DirectXCommon::GetInstance();
	//debugText = DebugText::GetInstance();

#pragma region シーンの初期化

	textureHandle = TextureManager::Load("Texture.jpg");
	sprite = Sprite::Create(textureHandle, {100, 50});

	/*model = std::unique_ptr<Model>(FbxLoader::GetInstance()->LoadModeFromFile("cube"));

	modelObject = std::make_unique<Object3D>();
	modelObject->Initialize();
	modelObject->SetModel(model.get());*/

	worldTransform.Initialize();
	viewProjection.Initialize();

#pragma endregion

}

void DirectXGame::Update()
{
#pragma region シーン更新

	//debugText->Print("aaa", 100, 100);

	//modelObject->Update();

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

	//modelObject->Draw(commandList, worldTransform, viewProjection);


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
