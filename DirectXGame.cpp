#include "DirectXGame.h"


using namespace std;

void DirectXGame::Initialize()
{
	//基底クラスの初期化
	GameBase::Initialize();

#pragma region シーンの初期化

	//読み込み
	soundManager->LoadWave(0, "Resources/fanfare.wav");

	//スプライト共通テクスチャ読込
	textureManager->LoadTexture(1, L"Resources/Texture.jpg");
	textureManager->LoadTexture(2, L"Resources/Texture2.jpg");

	//スプライト生成
	for(int i = 0; i < TextureNum; i++)
	{
		sprite[i] = make_unique<Sprite>();
		sprite[i]->Initialize(i+1);
		sprite[i]->SetPosition({float(100 + i*200), float(100)});
		sprite[i]->SetSize({100,50});
		sprite[i]->SetAnchorpoint({0.5f,0.5f});
		sprite[i]->SetTextureRect(0.f, 0.f, 400.f, 400.f);
	}


	//オブジェクト生成
	for(int i = 0; i < ObjectNum; i++)
	{
		object[i] = make_unique<GeometryObject3D>();
		object[i]->Initialize();

		object[i]->SetTexNumber(i%2 + 1);
		object[i]->SetRotation({0, 0, float(-0.5 + i*0.25)});
		object[i]->SetPosition({float(-50 + i*25), 0, 0});
	}
	
	//FBX モデル名を指定してファイル読み込み
	//FbxLoader::GetInstance()->LoadModeFromFile("cube");
	model = std::unique_ptr<Model>(FbxLoader::GetInstance()->LoadModeFromFile("dolly"));

	//3Dオブジェクト生成とモデルのセット
	modelObject = make_unique<Object3D>();
	modelObject->Initialize();
	modelObject->SetModel(model.get());
	modelObject->PlayAnimation();
	modelObject->SetScale({5.f, 5.f, 5.f});


#pragma endregion

}

void DirectXGame::Update()
{
	//基底クラスの更新
	GameBase::Update();

#pragma region シーン更新


	//カメラ
	if(input->Push(DIK_A) || input->Push(DIK_D))
	{
		if(input->Push(DIK_A))
		{
			camera->CameraMovement({-2.f,0,0});
		}
		else if(input->Push(DIK_D))
		{
			camera->CameraMovement({2.f,0,0});
		}
	}
	else if(input->Push(DIK_W) || input->Push(DIK_S))
	{
		if(input->Push(DIK_S))
		{
			camera->CameraMovement({0, -2.f,0});
		}
		else if(input->Push(DIK_W))
		{
			camera->CameraMovement({0,2.f,0});
		}
	}

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

	//カメラ更新
	camera->Update();

	//更新
	for(int i = 0; i < ObjectNum; i++)
	{
		object[i]->Update();
	}

	modelObject->Update();

	//サウンド
	if(input->Push(DIK_SPACE))
	{
		//再生
		soundManager->PlayWave(0);
	}

	//デバックテキスト
	debugText->Print("t", 200, 200, 4.0f);


#pragma endregion
}

void DirectXGame::Draw()
{
	//DirectXCommon前処理
	dxCommon->BeginDraw();


	//3D描画
	for(int i = 0; i < ObjectNum; i++)
	{
		object[i]->Draw(dxCommon->GetCommandList());
	}

	modelObject->Draw(dxCommon->GetCommandList());

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
	//基底クラスの解放
	GameBase::Finalize();

	//delete modelObject;
	//delete model;
}
