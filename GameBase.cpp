#include "GameBase.h"
#include "TextureManager.h"

void GameBase::Run()
{
	//初期化
	Initialize();

	while (true)
	{
		if(winApp->ProcessMessage())
		{
			break;
		}

		//更新
		Update();

		//描画
		Draw();
	}
	//解放
	Finalize();
}

void GameBase::Initialize()
{
#pragma region Windowsアプリ初期化
	
	winApp = WinApp::GetInstance();
	winApp->CreateGameWindow();

	dxCommon = DirectXCommon::GetInstance();
	dxCommon->Initialize(winApp);

#pragma endregion

#pragma region DirectX初期化ここから

	

#pragma endregion


#pragma region 汎用的機能の初期化
	
	TextureManager::GetInstance()->Initialize(dxCommon->GetDevice());
	TextureManager::Load("white1x1.png");

#pragma endregion

	//ゲームシーン初期化
	gameScene = new DirectXGame();
	gameScene->Initialize();
}

void GameBase::Update()
{
	gameScene->Update();
}

void GameBase::Draw()
{
	dxCommon->PreDraw();

	gameScene->Draw();

	dxCommon->PostDraw();
}

void GameBase::Finalize()
{
	delete gameScene;
	gameScene = nullptr;

	winApp->TerminateGameWindow();
}
