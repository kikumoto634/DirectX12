#include "WinApp.h"

//基本(初期化)
#include "DirectXCommon.h"
//#include <cassert>
//幾何学モデル
#include "GeometryModel.h"
//幾何学オブジェクト
#include "GeometryObject3D.h"
//キーボード入力
#include "Input.h"
//DirectXTex導入
#include "DirectXTex.h"
//テクスチャマネージャー
#include "TextureManager.h"
//ComPtrスマートポインタ
#include <wrl.h>
//スプライト
#include "Sprite.h"
//サウンド
#include "SoundManager.h"
//デバックテキスト
#include "DebugText.h"

//更新
//void UpdateObject3d(Object3d* object, XMMATRIX& matView, XMMATRIX& matProjection);


//WindowsAPIオブジェクト
WinApp* winApp = nullptr;
//DirectXオブジェクト
DirectXCommon* dxCommon = nullptr;
//テクスチャマネージャー
TextureManager* textureManager = nullptr;
//スプライト
const int TextureNum = 2;
Sprite* sprite = nullptr;
Sprite* sprite2 = nullptr;

//モデル
GeometryModel* geometryModel = nullptr;
//オブジェクト
GeometryObject3D* object = nullptr;


//デバック
DebugText* debugText = nullptr;

//サウンドマネージャー
SoundManager* soundManager = nullptr;

/// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE,LPSTR,int)
{
	//WindowsAPI
	winApp = new WinApp();
	//WinApp初期化
	winApp->Initialize();

	//DirectX
	dxCommon = new DirectXCommon();
	//初期化
	dxCommon->Initialize(winApp);

	//テクスチャマネージャー
	textureManager = new TextureManager();
	//初期化
	textureManager->Initialize(dxCommon);

	/// <summary>
	/// DirectX12 初期化処理 ここから
	/// </summary>
	
	/// <summary>
	/// 初期化変数
	/// </summary>

	//スプライト共通テクスチャ読込
	textureManager->LoadTexture(0, L"Resources/Texture.jpg");
	textureManager->LoadTexture(1, L"Resources/Texture2.jpg");

	//モデル初期化
	geometryModel = new GeometryModel();
	geometryModel->Initialize(dxCommon, textureManager, 0);

	////サウンド
	soundManager = new SoundManager();
	soundManager->Initialize();

	//読み込み
	soundManager->LoadWave(0, "Resources/fanfare.wav");

	///DirectInPut
	Input* input = new Input();
	//初期化
	input->Initialize(winApp->GetHwnd());


	/// <summary>
	/// DirectX12 初期化処理 ここまで
	/// </summary>
	

	
	/// <summary>
	/// DirectX12 描画初期化処理 ここから
	/// </summary>

	////スプライト共通データ生成
	Sprite::StaticInitialize(dxCommon, textureManager);

	///幾何学オブジェクト共通でーた生成
	GeometryObject3D::StaticInitialize(dxCommon, geometryModel);

	//デバックテキスト世のテクスチャ番号
	const int debugTextTexNumber = 2;
	textureManager->LoadTexture(debugTextTexNumber, L"Resources/texfont.png");

	//デバックテキスト初期化
	debugText = new DebugText();
	debugText->Initialize(debugTextTexNumber);


	//スプライト生成
	sprite = new Sprite();
	sprite->Initialize(0);
	sprite->SetPosition({100, 100});
	sprite->SetSize({100,50});
	sprite->SetAnchorpoint({0.5f,0.5f});
	sprite->SetTextureRect(0.f, 0.f, 400.f, 400.f);

	sprite2 = new Sprite();
	sprite2->Initialize(1);
	sprite2->SetPosition({300, 100});
	sprite2->SetSize({100,50});
	sprite2->SetAnchorpoint({0.5f,0.5f});
	sprite2->SetTextureRect(0.f, 0.f, 400.f, 400.f);


	//オブジェクト生成
	object = new GeometryObject3D();
	object->Initialize();


	/// <summary>
	/// DirectX12 描画初期化処理 ここまで
	/// </summary>
	 


	/// <summary>
	/// ゲームループ
	/// </summary>
	while(true)
	{
		//WindowsAPI毎フレーム処理
		bool msgExit = winApp->Update();
		if(msgExit)
		{
			break;
		}


		/// <summary>
		/// DirectX12 毎フレーム処理 ここから
		/// </summary>

		///キーボード情報の取得開始
		input->Update();

		if(input->Push(DIK_1) || input->Push(DIK_2)){
			XMFLOAT2 pos = sprite->GetPosition();
			
			if(input->Push(DIK_1)){
				pos.x -= 2.0f;
			}
			if(input->Push(DIK_2)){
				pos.x += 2.0f;
			}
			
			sprite->SetPosition(pos);
		}


		////入力check
		//if(input->Push(DIK_D) || input->Push(DIK_A))
		//{
		//	if(input->Push(DIK_D))angle += XMConvertToRadians(1.0f);
		//	else if(input->Push(DIK_A))angle -= XMConvertToRadians(1.0f);

		//	//angleラジアンだけy軸まわりに回転、半径は-100
		//	eye.x = -100 * sinf(angle);
		//	eye.z = -100 * cosf(angle);
		//	matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
		//}



		//if(input->Push(DIK_UP) || input->Push(DIK_DOWN) || input->Push(DIK_LEFT) || input->Push(DIK_RIGHT))
		//{
		//	if(input->Push(DIK_UP))
		//	{
		//		object3ds[0].position.y += 1.0f;
		//	}
		//	else if(input->Push(DIK_DOWN))
		//	{
		//		object3ds[0].position.y -= 1.0f;
		//	}

		//	if(input->Push(DIK_LEFT))
		//	{
		//		object3ds[0].position.x -= 1.0f;
		//	}
		//	else if(input->Push(DIK_RIGHT))
		//	{
		//		object3ds[0].position.x += 1.0f;
		//	}
		//}

		////更新処理
		//for(size_t i = 0; i < _countof(object3ds); i++)
		//{
		//	UpdateObject3d(&object3ds[i], matView, matProjection);
		//}
		object->Update();


		if(input->Push(DIK_SPACE))
		{
			//再生
			soundManager->PlayWave(0);
		}


		//デバックテキスト
		debugText->Print("t", 200, 200, 4.0f);


		//DirectXCommon前処理
		dxCommon->BeginDraw();


		//3D描画
		object->Draw(dxCommon->GetCommandList());


		//スプライト
		Sprite::SetPipelineState(dxCommon->GetCommandList());
		sprite->Draw(dxCommon->GetCommandList());
		sprite2->Draw(dxCommon->GetCommandList());

		debugText->DrawAll(dxCommon->GetCommandList());

		//DirectXCommon描画後処理
		dxCommon->EndDraw();
		
		/// <summary>
		/// DirectX12 毎フレーム処理 ここまで
		/// </summary>
	}
	delete object;
	delete sprite;
	delete sprite2;
	delete debugText;
	GeometryObject3D::StaticFinalize();
	Sprite::StaticFinalize();
	delete input;
	delete soundManager;
	delete geometryModel;
	delete textureManager;
	delete dxCommon;
	//ゲームウィンドウ破棄
	delete winApp;

	return 0;
}



//更新
//void UpdateObject3d(Object3d *object, XMMATRIX &matView, XMMATRIX &matProjection)
//{
//	XMMATRIX matScale, matRot, matTrans;
//
//	//スケール、回転、平行移動行列の計算
//	matScale = XMMatrixScaling(object->scale.x, object->scale.y, object->scale.z);
//	matRot = XMMatrixIdentity();
//	matRot *= XMMatrixRotationZ(object->rotation.z);
//	matRot *= XMMatrixRotationX(object->rotation.x);
//	matRot *= XMMatrixRotationY(object->rotation.y);
//	matTrans = XMMatrixTranslation(object->position.x, object->position.y, object->position.z);
//
//	//ワールド行列の合成
//	object->matWorld = XMMatrixIdentity();	//変形をリセット
//	object->matWorld *= matScale;			//ワールド行列にスケーリングを反映
//	object->matWorld *= matRot;				//ワールド行列に回転を反映
//	object->matWorld *= matTrans;			//ワールド行列に平行移動を反映
//
//	//親オブジェクトの存在
//	if(object->parent != nullptr)
//	{
//		//親オブジェクトのワールド行列を掛ける
//		object->matWorld *= object->parent->matWorld;
//	}
//
//
//	//定数バッファへのデータ転送
//	//値を書き込むと自動的に転送される
//	object->constBuffer->color = object->color;
//	object->constBuffer->mat = object->matWorld * matView *matProjection;
//}
