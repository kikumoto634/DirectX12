#include "WinApp.h"

//基本(初期化)
#include "DirectXCommon.h"
#include <cassert>
////図形描画
#include "GeometryModel.h"
//D3Dコンパイラのインクルード
#include <d3dcompiler.h>
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

//音声
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;
using namespace Microsoft::WRL;

//CD3DX12ヘルパー構造体
//#include <d3dx12.h>


//定数バッファ用データ構造体(3D変換行列
struct ConstBufferData{
	XMMATRIX mat;	//3D変換行列
	XMFLOAT4 color;	//色(RGBA)
};

//3Dオブジェクト型
struct Object3d
{
	//マッピング用ポインタ
	ConstBufferData* constBuffer = nullptr;
	//色
	XMFLOAT4 color = {1.0f, 1.0f, 1.0f, 1.0f};
	//定数バッファ(行列用)
	ComPtr<ID3D12Resource> constBuffData = nullptr;
	//アフィン変換
	XMFLOAT3 scale = {1.0f, 1.0f, 1.0f};
	XMFLOAT3 rotation = {0.0f, 0.0f, 0.0f};
	XMFLOAT3 position = {0.0f, 0.0f, 0.0f};
	//ワールド変換行列
	XMMATRIX matWorld;
	//親オブジェクトへのポインタ
	Object3d* parent = nullptr;
};

//パイプラインセット
struct PipelineSet
{
	//パイプラインステートオブジェクト
	ComPtr<ID3D12PipelineState> pipelinestate;
	//ルートシグネチャ
	ComPtr<ID3D12RootSignature> rootsignature;
};


//3Dオブジェクト用パイプライン生成
PipelineSet Object3dCreateGraphicsPipeline(ID3D12Device* device);

//3D共通グラフィックスコマンドのセット
void Object3DCommonBeginDraw(ID3D12GraphicsCommandList* commandList, const PipelineSet& pipelineSet);

//3Dオブジェクト初期化
void InitializeObject3d(Object3d* object, ID3D12Device* device);

//更新
void UpdateObject3d(Object3d* object, XMMATRIX& matView, XMMATRIX& matProjection);

//描画
void DrawObject3d(Object3d* object, ID3D12GraphicsCommandList* commandList);


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
	HRESULT result;

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

	//3Dオブジェクト用パイプライン生成
	PipelineSet object3dPipelineSet = Object3dCreateGraphicsPipeline(dxCommon->GetDevice());


	//定数バッファ Mat	
	//透視投影
	XMMATRIX matProjection;
	//ビュー変換行列
	XMMATRIX matView;
	XMFLOAT3 eye = {0.0f, 0.0f, -100.0f};	//視点座標
	XMFLOAT3 target= {0, 0, 0};//注視点座標
	XMFLOAT3 up = {0, 1, 0};	//上方向ベクトル

	//3Dオブジェクト数
	const size_t kObjectCount = 2;
	//3dオブジェクトの配列
	Object3d object3ds[kObjectCount];
		
	//配列内の全オブジェクトに対して
	for(size_t i = 0; i < _countof(object3ds); i++)
	{
		//初期化
		object3ds[0].color = {0.8f, 0.8f, 0.8f, 1.0f};
		object3ds[1].color = {0.2f, 0.2f, 0.2f, 1.0f};
		InitializeObject3d(&object3ds[i], dxCommon->GetDevice());

		//親子構造体
		//先頭以外
		if(i > 0)
		{
			//一つ前のオブジェクトを親とする
			object3ds[i].parent = &object3ds[i - 1];

			//Scale
			object3ds[i].scale = {1.f, 1.f, 1.f};
			//rotation
			object3ds[i].rotation = {0.0f, 0.0f, XMConvertToRadians(45.0f)};
			//position
			object3ds[i].position = {0.0f, 0.0f, 4.0f};
		}
	}

	

	//透視投影
	matProjection = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(45.0f),	//上下画角45°
		(float)WinApp::window_width / WinApp::window_height,			//aspect比(画面横幅/画面縦幅)
		0.1f, 1000.0f				//前端、奥端
	);

	//ビュー変換行列
	matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));	


	/// <summary>
	/// DirectX12 描画初期化処理 ここまで
	/// </summary>
	 

	float angle = 0.0f;//カメラの回転角


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


		//入力check
		if(input->Push(DIK_D) || input->Push(DIK_A))
		{
			if(input->Push(DIK_D))angle += XMConvertToRadians(1.0f);
			else if(input->Push(DIK_A))angle -= XMConvertToRadians(1.0f);

			//angleラジアンだけy軸まわりに回転、半径は-100
			eye.x = -100 * sinf(angle);
			eye.z = -100 * cosf(angle);
			matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
		}



		if(input->Push(DIK_UP) || input->Push(DIK_DOWN) || input->Push(DIK_LEFT) || input->Push(DIK_RIGHT))
		{
			if(input->Push(DIK_UP))
			{
				object3ds[0].position.y += 1.0f;
			}
			else if(input->Push(DIK_DOWN))
			{
				object3ds[0].position.y -= 1.0f;
			}

			if(input->Push(DIK_LEFT))
			{
				object3ds[0].position.x -= 1.0f;
			}
			else if(input->Push(DIK_RIGHT))
			{
				object3ds[0].position.x += 1.0f;
			}
		}

		//更新処理
		for(size_t i = 0; i < _countof(object3ds); i++)
		{
			UpdateObject3d(&object3ds[i], matView, matProjection);
		}

		if(input->Push(DIK_SPACE))
		{
			//再生
			soundManager->PlayWave(0);
		}


		//デバックテキスト
		debugText->Print("t", 200, 200, 4.0f);


		//DirectXCommon前処理
		dxCommon->BeginDraw();


		//3Dobject
		Object3DCommonBeginDraw(dxCommon->GetCommandList(), object3dPipelineSet);

		//全オブジェクトについて処理
		for(size_t i = 0; i < _countof(object3ds); i++)
		{
			DrawObject3d(&object3ds[i], dxCommon->GetCommandList());
		}


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
	delete debugText;
	delete sprite;
	delete sprite2;
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



//3Dオブジェクト用パイプラインセット
PipelineSet Object3dCreateGraphicsPipeline(ID3D12Device* device)
{
	HRESULT result;

	///頂点シェーダーfileの読み込みとコンパイル
	ComPtr<ID3DBlob> vsBlob ;			//頂点シェーダーオブジェクト
	ComPtr<ID3DBlob> psBlob ;			//ピクセルシェーダーオブジェクト
	ComPtr<ID3DBlob> errorBlob ;		//エラーオブジェクト

	//頂点シェーダーの読み込みコンパイル
	result = D3DCompileFromFile(
		L"BasicVS.hlsl",		//シェーダーファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,	//インクルード可能にする
		"main", "vs_5_0",					//エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,	//デバック用設定
		0,
		&vsBlob, &errorBlob);
	//エラーなら
	if(FAILED(result)){
		//errorBlobからエラー内容をstring型にコピー
		std::string error;
		error.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
					errorBlob->GetBufferSize(),
					error.begin());
		error += "\n";
		//エラー内容を出力ウィンドウに表示
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	//ピクセルシェーダーの読み込みコンパイル
	result = D3DCompileFromFile(
		L"BasicPS.hlsl",		//シェーダーファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,	//インクルード可能にする
		"main", "ps_5_0",					//エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,	//デバック用設定
		0,
		&psBlob, &errorBlob);
	//エラーなら
	if(FAILED(result)){
		//errorBlobからエラー内容をstring型にコピー
		std::string error;
		error.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
					errorBlob->GetBufferSize(),
					error.begin());
		error += "\n";
		//エラー内容を出力ウィンドウに表示
		OutputDebugStringA(error.c_str());
		assert(0);
	}


	///頂点レイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
	
		{//xyz座標
			"POSITION",										//セマンティック名
			0,												//同じセマンティック名が複数あるときに使うインデックス
			DXGI_FORMAT_R32G32B32_FLOAT,					//要素数とビット数を表す (XYZの3つでfloat型なのでR32G32B32_FLOAT)
			0,												//入力スロットインデックス
			D3D12_APPEND_ALIGNED_ELEMENT,					//データのオフセット値 (D3D12_APPEND_ALIGNED_ELEMENTだと自動設定)
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,		//入力データ種別 (標準はD3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA)
			0												//一度に描画するインスタンス数
		},
		{//法線ベクトル
			"NORMAL",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
		{//uv座標
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
	};

	///ルートパラメータ
	//デスクリプタレンジの設定
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV{};
	descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);


	//設定
	////定数バッファ 0番
	CD3DX12_ROOT_PARAMETER rootParam[2] = {};
	////定数　0番 material
	rootParam[0].InitAsConstantBufferView(0);
	////テクスチャレジスタ 0番
	rootParam[1].InitAsDescriptorTable(1, &descRangeSRV);


	///<summmary>
	///グラフィックスパイプライン
	///<summary/>
	
	//グラフィックスパイプライン設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};
	//シェーダー設定
	pipelineDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	pipelineDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
	
	//サンプルマスク設定
	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;	//標準設定
	//ラスタライザ設定 背面カリング	ポリゴン内塗りつぶし	深度クリッピング有効
	pipelineDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//ブレンドステート
	//レンダーターゲットのブレンド設定
	D3D12_RENDER_TARGET_BLEND_DESC& blenddesc = pipelineDesc.BlendState.RenderTarget[0];
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;	//RGBAすべてのチャンネルを描画
	//共通設定
	blenddesc.BlendEnable = true;						//ブレンドを有効にする
	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;		//加算
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;			//ソースの値を100% 使う	(ソースカラー			 ： 今から描画しようとしている色)
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;		//デストの値を  0% 使う	(デスティネーションカラー： 既にキャンバスに描かれている色)
	//各種設定
	blenddesc.BlendOp = D3D12_BLEND_OP_ADD;	//設定
	blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;			//ソースの値を 何% 使う
	blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;	//デストの値を 何% 使う
	//頂点レイアウト設定
	pipelineDesc.InputLayout.pInputElementDescs = inputLayout;
	pipelineDesc.InputLayout.NumElements = _countof(inputLayout);
	//図形の形状設定 (プリミティブトポロジー)
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//その他設定
	pipelineDesc.NumRenderTargets = 1;		//描画対象は一つ
	pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;	//0~255指定のRGBA
	pipelineDesc.SampleDesc.Count = 1;	//1ピクセルにつき1回サンプリング
	//デプスステンシルステートの設定	(深度テストを行う、書き込み許可、深度がちいさければ許可)
	pipelineDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;	//深度値フォーマット

	///テクスチャサンプラー
	//設定
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);


	//パイプラインとルートシグネチャのセット
	PipelineSet pipelineSet;


	//ルートシグネチャ (テクスチャ、定数バッファなどシェーダーに渡すリソース情報をまとめたオブジェクト)
	//設定
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Init_1_0(_countof(rootParam), rootParam,1, &samplerDesc,D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	//シリアライズ
	ComPtr<ID3DBlob> rootSigBlob;
	result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob,&errorBlob);
	assert(SUCCEEDED(result));
	result = dxCommon->GetDevice()->CreateRootSignature(0,rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(),IID_PPV_ARGS(&pipelineSet.rootsignature));
	assert(SUCCEEDED(result));
	//パイプラインにルートシグネチャをセット
	pipelineDesc.pRootSignature = pipelineSet.rootsignature.Get();

	//パイプラインステート (グラフィックスパイプラインの設定をまとめたのがパイプラインステートオブジェクト(PSO))
	//パイプラインステートの生成
	result = dxCommon->GetDevice()->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipelineSet.pipelinestate));
	assert(SUCCEEDED(result));

	//パイプラインとルートシグネチャを返す
	return pipelineSet;
}


//3DObject共通グラフィックスコマンドのセット
void Object3DCommonBeginDraw(ID3D12GraphicsCommandList* commandList, const PipelineSet& pipelineSet)
{
	//パイプラインステートの設定
	commandList->SetPipelineState(pipelineSet.pipelinestate.Get());
	//ルートシグネチャの設定
	commandList->SetGraphicsRootSignature(pipelineSet.rootsignature.Get());
	//プリミティブ形状を設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}



//初期化
void InitializeObject3d(Object3d *object, ID3D12Device* device)
{
	HRESULT result;
	//定数バッファ生成
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xff) & ~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&object->constBuffData)
		);
	assert(SUCCEEDED(result));

	//定数バッファのマッピング
	result = object->constBuffData->Map(0,nullptr, (void**)&object->constBuffer);
	assert(SUCCEEDED(result));

	////値を書き込むと自動的に転送される(色の初期化)
	//object->constBuffer->color = object->color;
}

//更新
void UpdateObject3d(Object3d *object, XMMATRIX &matView, XMMATRIX &matProjection)
{
	XMMATRIX matScale, matRot, matTrans;

	//スケール、回転、平行移動行列の計算
	matScale = XMMatrixScaling(object->scale.x, object->scale.y, object->scale.z);
	matRot = XMMatrixIdentity();
	matRot *= XMMatrixRotationZ(object->rotation.z);
	matRot *= XMMatrixRotationX(object->rotation.x);
	matRot *= XMMatrixRotationY(object->rotation.y);
	matTrans = XMMatrixTranslation(object->position.x, object->position.y, object->position.z);

	//ワールド行列の合成
	object->matWorld = XMMatrixIdentity();	//変形をリセット
	object->matWorld *= matScale;			//ワールド行列にスケーリングを反映
	object->matWorld *= matRot;				//ワールド行列に回転を反映
	object->matWorld *= matTrans;			//ワールド行列に平行移動を反映

	//親オブジェクトの存在
	if(object->parent != nullptr)
	{
		//親オブジェクトのワールド行列を掛ける
		object->matWorld *= object->parent->matWorld;
	}


	//定数バッファへのデータ転送
	//値を書き込むと自動的に転送される
	object->constBuffer->color = object->color;
	object->constBuffer->mat = object->matWorld * matView *matProjection;
}

void DrawObject3d(Object3d *object, ID3D12GraphicsCommandList* commandList)
{
	//定数バッファビュー(CBVの設定コマンド)
	commandList->SetGraphicsRootConstantBufferView(0, object->constBuffData->GetGPUVirtualAddress());
	//モデル描画
	geometryModel->Draw(commandList);
}
