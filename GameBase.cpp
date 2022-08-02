#include "GameBase.h"
#include "FbxLoader.h"
#include "Object3D.h"

using namespace std;

void GameBase::Run()
{
	//初期化
	Initialize();

	while (true)
	{
		//終了フラグによってループを抜ける
		if(endFlag){
			break;
		}

		//更新
		Update();

		//描画
		Draw();
	}
}

void GameBase::Initialize()
{
#pragma region Windowsアプリ初期化
	//WindowsAPI
	winApp = make_unique<WinApp>();
	//WinApp初期化
	winApp->Initialize();
#pragma endregion

#pragma region DirectX初期化ここから

	//DirectX
	dxCommon = DirectXCommon::GetInstance();

#pragma endregion


#pragma region 汎用的機能の初期化
	//テクスチャマネージャー
	//textureManager = make_unique<TextureManager>();
	//初期化
	//textureManager->Initialize(dxCommon.get());

	//モデル初期化
	//geometryModel = make_unique<GeometryManager>();
	//geometryModel->Initialize(dxCommon.get(), textureManager.get());

	//カメラ初期化
	//camera = make_unique<Camera>();
	//camera->Initialize();

	///DirectInPut
	//input = make_unique<Input>();
	//初期化
	//input->Initialize(winApp->GetHwnd()); 

	////サウンド
	//soundManager = make_unique<SoundManager>();
	//soundManager->Initialize();

	////スプライト共通データ生成
	//Sprite::StaticInitialize(dxCommon.get(), textureManager.get());

	///幾何学オブジェクト共通初期化
	//GeometryObject3D::StaticInitialize(dxCommon.get(), geometryModel.get(), camera.get());

	//FBX初期化
	//FbxLoader::GetInstance()->Initialize(dxCommon->GetDevice());

	///デバイスのセット
	//Object3D::SetDevice(dxCommon->GetDevice());

	//カメラをセット
	//Object3D::SetCamera(camera.get());

	//グラフィックスパイプラインの生成
	//Object3D::CreateGraphicsPipeline();

	//デバックテキスト世のテクスチャ番号
	//const int debugTextTexNumber = 0;
	//textureManager->LoadTexture(debugTextTexNumber, L"Resources/texfont.png");

	//デバックテキスト初期化
	//debugText = make_unique<DebugText>();
	//debugText->Initialize(debugTextTexNumber);

#pragma endregion
}

void GameBase::Update()
{
	//WindowsAPI毎フレーム処理
	bool msgExit = winApp->Update();
	if(msgExit)
	{
		endFlag = true;
	}

	///キーボード情報の取得開始
	//input->Update();
}
