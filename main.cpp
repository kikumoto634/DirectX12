#include "WinApp.h"

//基本(初期化)
#include "DirectXCommon.h"
#include <cassert>
//図形描画
#include <DirectXMath.h>
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
//#include <xaudio2.h>
#include "SoundManager.h"
//ファイル読み込み
//#include <fstream>

//音声
#pragma comment(lib, "d3dcompiler.lib")
//#pragma comment(lib, "xaudio2.lib")

using namespace DirectX;
using namespace Microsoft::WRL;

//CD3DX12ヘルパー構造体
//#include <d3dx12.h>



//頂点データ構造体(3D)
struct Vertex
{
	XMFLOAT3 pos;	//xyz座標
	XMFLOAT3 normal;//法線ベクトル
	XMFLOAT2 uv;	//uv座標
};


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

////デバック用文字列クラスの定義
//class DebugText
//{
//public:
//
//	static const int maxCharCount = 256;
//	static const int fontWidth = 9;
//	static const int fontHeight = 18;
//	static const int fontLineCount = 14;
//
//private:
//	Sprite sprite[maxCharCount];
//	int spriteIndex = 0;
//};


//3Dオブジェクト用パイプライン生成
PipelineSet Object3dCreateGraphicsPipeline(ID3D12Device* device);

//3D共通グラフィックスコマンドのセット
void Object3DCommonBeginDraw(ID3D12GraphicsCommandList* commandList, const PipelineSet& pipelineSet, ID3D12DescriptorHeap* descHeap);

//3Dオブジェクト初期化
void InitializeObject3d(Object3d* object, ID3D12Device* device);

//更新
void UpdateObject3d(Object3d* object, XMMATRIX& matView, XMMATRIX& matProjection);

//描画
void DrawObject3d(Object3d* object, ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW& vbView, D3D12_INDEX_BUFFER_VIEW& ibView, ID3D12DescriptorHeap* srvHeap, UINT numIndices);


//WindowsAPIオブジェクト
WinApp* winApp = nullptr;
//DirectXオブジェクト
DirectXCommon* dxCommon = nullptr;
//テクスチャマネージャー
TextureManager* textureManager = nullptr;
//スプライト
const int TextureNum = 2;
Sprite* sprite = nullptr;

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

	////3DObject変数
	///頂点バッファ
	ComPtr<ID3D12Resource> vertBuff;
	///頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	///インデックスバッファ
	ComPtr<ID3D12Resource> indexBuff;
	///インデックスバッファビュー
	D3D12_INDEX_BUFFER_VIEW ibView{};


	////テクスチャ変数
	ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr;
	//テクスチャバッファの生成
	ComPtr<ID3D12Resource> texBuff01 ;

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
	 
	///頂点データ
	Vertex vertices[] = 
	{
		//前
		{{-5.0f, -5.0f, -5.0f}, {}, {0.0f, 1.0f}},
		{{-5.0f, +5.0f, -5.0f}, {}, {0.0f, 0.0f}},
		{{+5.0f, -5.0f, -5.0f}, {}, {1.0f, 1.0f}},
		{{+5.0f, +5.0f, -5.0f}, {}, {1.0f, 0.0f}},
		//後
		{{-5.0f, -5.0f, +5.0f}, {}, {0.0f, 1.0f}},
		{{-5.0f, +5.0f, +5.0f}, {}, {0.0f, 0.0f}},
		{{+5.0f, -5.0f, +5.0f}, {}, {1.0f, 1.0f}},
		{{+5.0f, +5.0f, +5.0f}, {}, {1.0f, 0.0f}},
		//左
		{{-5.0f, -5.0f, -5.0f}, {}, {0.0f, 1.0f}},
		{{-5.0f, -5.0f, +5.0f}, {}, {0.0f, 0.0f}},
		{{-5.0f, +5.0f, -5.0f}, {}, {1.0f, 1.0f}},
		{{-5.0f, +5.0f, +5.0f}, {}, {1.0f, 0.0f}},
		//右
		{{+5.0f, -5.0f, -5.0f}, {}, {0.0f, 1.0f}},
		{{+5.0f, -5.0f, +5.0f}, {}, {0.0f, 0.0f}},
		{{+5.0f, +5.0f, -5.0f}, {}, {1.0f, 1.0f}},
		{{+5.0f, +5.0f, +5.0f}, {}, {1.0f, 0.0f}},
		//下
		{{-5.0f, -5.0f, -5.0f}, {}, {0.0f, 1.0f}},
		{{+5.0f, -5.0f, -5.0f}, {}, {0.0f, 0.0f}},
		{{-5.0f, -5.0f, +5.0f}, {}, {1.0f, 1.0f}},
		{{+5.0f, -5.0f, +5.0f}, {}, {1.0f, 0.0f}},
		//上
		{{-5.0f, +5.0f, -5.0f}, {}, {0.0f, 1.0f}},
		{{+5.0f, +5.0f, -5.0f}, {}, {0.0f, 0.0f}},
		{{-5.0f, +5.0f, +5.0f}, {}, {1.0f, 1.0f}},
		{{+5.0f, +5.0f, +5.0f}, {}, {1.0f, 0.0f}},
	};

	///インデックスデータ
	uint16_t indices[] = 
	{
		//前
		0, 1, 2,
		2, 1, 3,
		//後
		5, 4, 6,
		5, 6, 7,
		//左
		8, 9, 10,
		10, 9, 11,
		//右
		13, 12, 14,
		13, 14, 15,
		//下
		16, 17, 18,
		18, 17, 19,
		//上
		21, 20, 22,
		21, 22, 23,
	};

	///法線計算
	for(int i = 0; i < _countof(indices)/3; i++)
	{//三角形一つごとに計算していく
		//三角形にインデックスを取り出して、一時的な変数を入れる
		uint16_t index0 = indices[i*3+0];
		uint16_t index1 = indices[i*3+1];
		uint16_t index2 = indices[i*3+2];
		//三角形を構成する頂点座標をベクトルに代入
		XMVECTOR p0 = XMLoadFloat3(&vertices[index0].pos);
		XMVECTOR p1 = XMLoadFloat3(&vertices[index1].pos);
		XMVECTOR p2 = XMLoadFloat3(&vertices[index2].pos);
		//p0->p1ベクトル、p0->p2ベクトルの計算	(ベクトル減算)
		XMVECTOR v1 = XMVectorSubtract(p1,p0);
		XMVECTOR v2 = XMVectorSubtract(p2,p0);
		//外積は両方から垂直なベクトル
		XMVECTOR normal = XMVector3Cross(v1,v2);
		//正規化(長さを1にする)
		normal = XMVector3Normalize(normal);
		//求めた法線を頂点データに代入
		XMStoreFloat3(&vertices[index0].normal,normal);
		XMStoreFloat3(&vertices[index1].normal,normal);
		XMStoreFloat3(&vertices[index2].normal,normal);
	}


	//頂点データ全体のサイズ = 頂点データ一つ分のサイズ * 頂点データの要素数
	UINT sizeVB = static_cast<UINT>(sizeof(vertices[0]) * _countof(vertices));

	///頂点バッファの確保
	//生成
	result = dxCommon->GetDevice()->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeVB),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&vertBuff)
		);
	assert(SUCCEEDED(result));

	///頂点バッファへのデータ転送
	//GPU状のバッファに対応した仮想メモリ(メインメモリ上)を取得
	Vertex* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	assert(SUCCEEDED(result));
	//全頂点に対して
	for(int i = 0; i < _countof(vertices); i++)
	{
		vertMap[i] = vertices[i];	//座標コピー
	}
	//繋がり解除
	vertBuff->Unmap(0, nullptr);


	///頂点バッファビューの作成(GPUに頂点バッファを教えるオブジェクト)
	//作成
	//GPU仮想アドレス
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	//頂点バッファのサイズ
	vbView.SizeInBytes = sizeVB;
	//頂点一つ分のデータサイズ
	vbView.StrideInBytes = sizeof(vertices[0]);


	///頂点インデックス
	//インデックスデータ全体のサイズ
	UINT sizeIB = static_cast<UINT>(sizeof(uint16_t) * _countof(indices));

	///インデックスバッファの生成
	//生成
	result = dxCommon->GetDevice()->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeIB),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&indexBuff)
		);
	assert(SUCCEEDED(result));

	///インデックスバッファへのデータ転送
	//マッピング
	uint16_t* indexMap = nullptr;
	result = indexBuff->Map(0, nullptr, (void**)&indexMap);
	assert(SUCCEEDED(result));
	//全インデックスに対して
	for(int i = 0; i < _countof(indices); i++)
	{
		indexMap[i] = indices[i];
	}
	//マッピング解除
	indexBuff->Unmap(0, nullptr);


	///インデックスバッファビューの作成(GPUにインデックスバッファを教えるオブジェクト)
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeIB;



	////スプライト共通データ生成
	Sprite::StaticInitialize(dxCommon, textureManager);

	//スプライト共通テクスチャ読込
	textureManager->LoadTexture(0, L"Resources/Texture.jpg");
	textureManager->LoadTexture(1, L"Resources/Texture2.jpg");

	//スプライト生成
	sprite = new Sprite();
	sprite->Initialize(0);
	sprite->SetPosition({100, 100});
	sprite->SetSize({100,50});
	sprite->SetAnchorpoint({0.5f,0.5f});

	//3Dオブジェクト用パイプライン生成
	PipelineSet object3dPipelineSet = Object3dCreateGraphicsPipeline(dxCommon->GetDevice());



	
	///画像ファイルの用意
	TexMetadata metadata{};
	ScratchImage scratchImg{};
	//WICテクスチャデータのロード
	result = LoadFromWICFile(
		L"Resources/Texture.jpg",
		WIC_FLAGS_NONE,
		&metadata, scratchImg);
	assert(SUCCEEDED(result));


	//ミップマップの生成
	ScratchImage mipChain{};
	//生成
	result = GenerateMipMaps(
		scratchImg.GetImages(), 
		scratchImg.GetImageCount(), 
		scratchImg.GetMetadata(),
		TEX_FILTER_DEFAULT, 
		0, 
		mipChain
	);
	if(SUCCEEDED(result))
	{
		scratchImg = std::move(mipChain);
		metadata = scratchImg.GetMetadata();
	}

	//フォーマットを書き換える
	//読み込んだディフューズテクスチャをSRGBとして扱う
	metadata.format = MakeSRGB(metadata.format);


	///テクスチャバッファ設定
	//ヒープ設定
	D3D12_HEAP_PROPERTIES textureHandleProp{};
	textureHandleProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	textureHandleProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	textureHandleProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	//リソース設定
	CD3DX12_RESOURCE_DESC textureResourceDesc01 = CD3DX12_RESOURCE_DESC::Tex2D
		(
			metadata.format,
			metadata.width,
			(UINT)metadata.height,
			(UINT16)metadata.arraySize,
			(UINT16)metadata.mipLevels
		);

	//テクスチャバッファの生成
	result= dxCommon->GetDevice()->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0),
			D3D12_HEAP_FLAG_NONE,
			&textureResourceDesc01,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&texBuff01)
		);
	assert(SUCCEEDED(result));


	//テクスチャバッファへのデータ転送
	//全ミップマップについて
	for(size_t i = 0; i < metadata.mipLevels; i++)
	{
		//ミップマップレベルを指定してイメージを取得
		const Image* img = scratchImg.GetImage(i, 0, 0);
		//テクスチャバッファにデータ転送
		result = texBuff01->WriteToSubresource(
			(UINT)i,				
			nullptr,				//全領域へコピー
			img->pixels,			//元データアドレス
			(UINT)img->rowPitch,	//一ラインサイズ
			(UINT)img->slicePitch	//一枚サイズ
		);
		assert(SUCCEEDED(result));
	}


	///デスクリプタヒープ生成
	//SRVの最大個数
	const size_t kMaxSRVCount = 2056;
	//デスクリプタヒープの設定
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;	//シェーダーから見えるように
	srvHeapDesc.NumDescriptors = kMaxSRVCount;
	//設定をもとにSRV用デスクリプタヒープを生成
	result = dxCommon->GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap));
	assert(SUCCEEDED(result));

	///シェーダリソースビューの作成
	//設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};	//設定構造体
	srvDesc.Format = textureResourceDesc01.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;	//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = textureResourceDesc01.MipLevels;

	//ハンドルの指す位置にシェーダーリソースビューの作成
	dxCommon->GetDevice()->CreateShaderResourceView
	(
		texBuff01.Get(),
		&srvDesc,
		CD3DX12_CPU_DESCRIPTOR_HANDLE//SRVヒープの先頭ハンドルを取得
			(
				srvHeap->GetCPUDescriptorHandleForHeapStart(),
				0,
				dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
			)
	);


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


		//DirectXCommon前処理
		dxCommon->BeginDraw();


		//3Dobject
		Object3DCommonBeginDraw(dxCommon->GetCommandList(), object3dPipelineSet, srvHeap.Get());

		//全オブジェクトについて処理
		for(size_t i = 0; i < _countof(object3ds); i++)
		{
			DrawObject3d(&object3ds[i], dxCommon->GetCommandList(), vbView, ibView, srvHeap.Get(), _countof(indices));
		}


		//スプライト
		Sprite::SetPipelineState(dxCommon->GetCommandList());
		sprite->Draw(dxCommon->GetCommandList());

		//DirectXCommon描画後処理
		dxCommon->EndDraw();
		
		/// <summary>
		/// DirectX12 毎フレーム処理 ここまで
		/// </summary>
	}

	//XAudio2の解放
	//xAudio2.Reset();
	//音声データの開放
	//SoundUnload(&soundData1);
	Sprite::StaticFinalize();
	delete input;
	delete soundManager;
	delete sprite;
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
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV;
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
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
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
void Object3DCommonBeginDraw(ID3D12GraphicsCommandList* commandList, const PipelineSet& pipelineSet, ID3D12DescriptorHeap* descHeap)
{
	//パイプラインステートの設定
	commandList->SetPipelineState(pipelineSet.pipelinestate.Get());
	//ルートシグネチャの設定
	commandList->SetGraphicsRootSignature(pipelineSet.rootsignature.Get());
	//プリミティブ形状を設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//テクスチャ用デスクリプタヒープの設定
	ID3D12DescriptorHeap* ppHeaps[] = {descHeap};
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
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

void DrawObject3d(Object3d *object, ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW &vbView, D3D12_INDEX_BUFFER_VIEW &ibView, ID3D12DescriptorHeap* srvHeap,UINT numIndices)
{

	//頂点バッファの設定
	commandList->IASetVertexBuffers(0, 1, &vbView);
	//インデックスバッファの設定
	commandList->IASetIndexBuffer(&ibView);
	//定数バッファビュー(CBVの設定コマンド)
	commandList->SetGraphicsRootConstantBufferView(0, object->constBuffData->GetGPUVirtualAddress());
	//シェーダリソースビューをセット
	dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable
		(
			1, 
			CD3DX12_GPU_DESCRIPTOR_HANDLE//SRVヒープの先頭ハンドルを取得
			(
				srvHeap->GetGPUDescriptorHandleForHeapStart(),
				0,
				dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
			)
		);

	//描画コマンド
	commandList->DrawIndexedInstanced(numIndices,1, 0, 0, 0);
}
