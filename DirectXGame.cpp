#include "DirectXGame.h"

using namespace std;

void DirectXGame::Initialize()
{
#pragma region Windowsアプリ初期化
	//WindowsAPI
	winApp = make_unique<WinApp>();
	//WinApp初期化
	winApp->Initialize();
#pragma endregion

#pragma region DirectX初期化ここから

	//DirectX
	dxCommon = make_unique<DirectXCommon>();
	//初期化
	dxCommon->Initialize(winApp.get());

#pragma endregion


#pragma region 汎用的機能の初期化
	//テクスチャマネージャー
	textureManager = make_unique<TextureManager>();
	//初期化
	textureManager->Initialize(dxCommon.get());

	//モデル初期化
	geometryModel = make_unique<GeometryModel>();
	geometryModel->Initialize(dxCommon.get(), textureManager.get(), 0);

	///DirectInPut
	input = make_unique<Input>();
	//初期化
	input->Initialize(winApp->GetHwnd()); 

	////サウンド
	soundManager = make_unique<SoundManager>();
	soundManager->Initialize();

	////スプライト共通データ生成
	Sprite::StaticInitialize(dxCommon.get(), textureManager.get());

	///幾何学オブジェクト共通初期化
	GeometryObject3D::StaticInitialize(dxCommon.get(), geometryModel.get());

	//デバックテキスト世のテクスチャ番号
	const int debugTextTexNumber = 2;
	textureManager->LoadTexture(debugTextTexNumber, L"Resources/texfont.png");

	//デバックテキスト初期化
	debugText = make_unique<DebugText>();
	debugText->Initialize(debugTextTexNumber);

#pragma endregion


#pragma region シーンの初期化

	//読み込み
	soundManager->LoadWave(0, "Resources/fanfare.wav");

	//スプライト共通テクスチャ読込
	textureManager->LoadTexture(0, L"Resources/Texture.jpg");
	textureManager->LoadTexture(1, L"Resources/Texture2.jpg");


	//スプライト生成
	for(int i = 0; i < TextureNum; i++)
	{
		sprite[i] = make_unique<Sprite>();
		sprite[i]->Initialize(i);
		sprite[i]->SetPosition({float(100 + i*200), float(100)});
		sprite[i]->SetSize({100,50});
		sprite[i]->SetAnchorpoint({0.5f,0.5f});
		sprite[i]->SetTextureRect(0.f, 0.f, 400.f, 400.f);
	}


	//オブジェクト生成
	const double M_PI = 3.14159265;
	for(int i = 0; i < ObjectNum; i++)
	{
		object[i] = make_unique<GeometryObject3D>();
		object[i]->Initialize();

		object[i]->SetRotation({0, 0, float(-2 + i)});
		object[i]->SetPosition({float(-50 + i*25), 0, 0});
	}

	
#pragma endregion

}

void DirectXGame::Update()
{
	//WindowsAPI毎フレーム処理
	bool msgExit = winApp->Update();
	if(msgExit)
	{
		endFlag = true;
	}

	///キーボード情報の取得開始
	input->Update();


	//スプライト
	if(input->Push(DIK_1) || input->Push(DIK_2)){
		XMFLOAT2 pos = sprite[0]->GetPosition();
			
		if(input->Push(DIK_1)){
			pos.x -= 2.0f;
		}
		if(input->Push(DIK_2)){
			pos.x += 2.0f;
		}
			
		sprite[0]->SetPosition(pos);
	}


	//オブジェクト
	if(input->Push(DIK_LEFT) || input->Push(DIK_RIGHT) || input->Push(DIK_UP) || input->Push(DIK_DOWN)){
		XMFLOAT3 pos = object[2]->GetPosition();
			
		if(input->Push(DIK_LEFT)){
			pos.x -= 1.0f;
		}
		else if(input->Push(DIK_RIGHT)){
			pos.x += 1.0f;
		}
			
		if(input->Push(DIK_UP))
		{
			pos.y += 1.f;
		}
		else if(input->Push(DIK_DOWN))
		{
			pos.y -= 1.f;
		}

		object[2]->SetPosition(pos);
	}
	for(int i = 0; i < ObjectNum; i++)
	{
		object[i]->Update();
	}


	//サウンド
	if(input->Push(DIK_SPACE))
	{
		//再生
		soundManager->PlayWave(0);
	}
}

void DirectXGame::Draw()
{
	//デバックテキスト
	debugText->Print("t", 200, 200, 4.0f);


	//DirectXCommon前処理
	dxCommon->BeginDraw();


	//3D描画
	for(int i = 0; i < ObjectNum; i++)
	{
		object[i]->Draw(dxCommon->GetCommandList());
	}

	//スプライト
	Sprite::SetPipelineState(dxCommon->GetCommandList());
	for(int i = 0; i < TextureNum; i++)
	{
		sprite[i]->Draw(dxCommon->GetCommandList());
	}

	debugText->DrawAll(dxCommon->GetCommandList());

	//DirectXCommon描画後処理
	dxCommon->EndDraw();
}

void DirectXGame::Finalize()
{
	//static
	GeometryObject3D::StaticFinalize();
	Sprite::StaticFinalize();
}
