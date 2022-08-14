#include "DirectXGame.h"


using namespace std;

void DirectXGame::Initialize()
{
	//基底クラスの初期化
	GameBase::Initialize();

#pragma region シーンの初期化

	//スプライト共通テクスチャ読込
	textureManager->LoadTexture(1, L"Resources/Texture.jpg");
	textureManager->LoadTexture(2, L"Resources/Texture2.jpg");


	playerObject = make_unique<GeometryObject3D>();
	player = make_unique<Player>();
	player->Initialize(input.get(), 1, playerObject.get());

	enemyObject = make_unique<GeometryObject3D>();
	enemy = make_unique<Enemy>();
	enemy->Initialize(2, enemyObject.get(), {0, 25, 500});

#pragma endregion

}

void DirectXGame::Update()
{
	//基底クラスの更新
	GameBase::Update();

#pragma region シーン更新

	//カメラ更新
	camera->Update();

	//更新
	player->Update();
	if(enemy)
	{
		enemy->Update();
	}

#pragma endregion
}

void DirectXGame::Draw()
{
	//DirectXCommon前処理
	dxCommon->BeginDraw();


	//3D描画
	player->Draw(dxCommon->GetCommandList());
	if(enemy)
	{
		enemy->Draw(dxCommon->GetCommandList());
	}

	//DirectXCommon描画後処理
	dxCommon->EndDraw();
}

void DirectXGame::Finalize()
{
	//基底クラスの解放
	GameBase::Finalize();

}
