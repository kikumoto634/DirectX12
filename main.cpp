//#include <Windows.h>
#include "WinApp.h"

//基本(初期化)
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

//アダプタの列挙
#include <vector>
#include <string>

//図形描画
#include <DirectXMath.h>
using namespace DirectX;

//D3Dコンパイラのインクルード
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

////キーボード入力
#include "Input.h"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

//DirectXTex導入
#include "DirectXTex.h"

//ComPtrスマートポインタ
#include <wrl.h>
using namespace Microsoft::WRL;

//CD3DX12ヘルパー構造体
#include <d3dx12.h>


//頂点データ構造体
struct Vertex
{
	XMFLOAT3 pos;	//xyz座標
	XMFLOAT3 normal;//法線ベクトル
	XMFLOAT2 uv;	//uv座標
};

//定数バッファ用データ構造体(マテリアル)
struct ConstBufferDataMaterial{
	XMFLOAT4 color;	//色(RGBA)
};

//定数バッファ用データ構造体(3D変換行列
struct ConstBufferDataTransform{
	XMMATRIX mat;	//3D変換行列
};

//3Dオブジェクト型
struct Object3d
{
	///定数バッファ Color
	//GPUリソースポインタ
	ComPtr<ID3D12Resource> constBufferMaterial = nullptr;
	//マッピング用ポインタ
	ConstBufferDataMaterial* constMapMaterial = nullptr;

	//色
	XMFLOAT4 color = {1.0f, 1.0f, 1.0f, 1.0f};

	//定数バッファ(行列用)
	ComPtr<ID3D12Resource> constBuffTransform = nullptr;
	//定数バッファマップ(行列用)
	ConstBufferDataTransform* constMapTransform = nullptr;
	//アフィン変換
	XMFLOAT3 scale = {1.0f, 1.0f, 1.0f};
	XMFLOAT3 rotation = {0.0f, 0.0f, 0.0f};
	XMFLOAT3 position = {0.0f, 0.0f, 0.0f};
	//ワールド変換行列
	XMMATRIX matWorld;
	//親オブジェクトへのポインタ
	Object3d* parent = nullptr;
};

//3Dオブジェクト初期化
void InitializeObject3d(Object3d* object, ID3D12Device* device);
void UpdateObject3d(Object3d* object, XMMATRIX& matView, XMMATRIX& matProjection);
void DrawObject3d(Object3d* object, ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW& vbView, D3D12_INDEX_BUFFER_VIEW& ibView, UINT numIndices);

//WindowsAPIオブジェクト
WinApp* winApp = nullptr;

/// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE,LPSTR,int)
{
	//WindowsAPI
	winApp = new WinApp();
	//WinApp初期化
	winApp->Initialize();

	/// <summary>
	/// DirectX12 初期化処理 ここから
	/// </summary>

	///デバックレイヤー
#ifdef _DEBUG
	//デバックレイヤーをオンに
	ID3D12Debug* debugController= nullptr;
	if(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))){
		debugController->EnableDebugLayer();
	}
#endif // _DEBUG


	/// <summary>
	/// 初期化変数
	/// </summary>
	HRESULT result;

	ComPtr<IDXGIFactory7> dxgiFactory= nullptr;
	ComPtr<ID3D12Device> device = nullptr;
	ComPtr<IDXGISwapChain4> swapChain = nullptr;
	ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	std::vector<ComPtr<ID3D12Resource>> backBuffers;
	ComPtr<ID3D12DescriptorHeap> dsvHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr;

	///アダプタ列挙
	//DXGIファクトリー
	result = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(result));

	//アダプターの列挙用
	std::vector<ComPtr<IDXGIAdapter4>> adapters;
	//ここに特定の名前を持つアダプターオブジェクトが入る
	ComPtr<IDXGIAdapter4> tmpAdapter = nullptr;

	//パフォーマンスが高いものから順に、すべてのアダプタを列挙する
	for(UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&tmpAdapter)) != DXGI_ERROR_NOT_FOUND; i++)
	{
		//動的配列に追加する
		adapters.push_back(tmpAdapter);
	}

	///アダプタの選別
	//打倒なアダプタを選別する
	for(size_t i = 0; i < adapters.size(); i++)
	{
		DXGI_ADAPTER_DESC3 adapterDesc;
		//アダプターの情報を取得する
		adapters[i]->GetDesc3(&adapterDesc);

		//ソフトウェアデバイスを回避
		if(!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
		{
			//デバイスを採用してループを抜ける
			tmpAdapter = adapters[i].Get();
			break;
		}
	}

	///デバイスの生成(1ゲームに一つ)
	//対応レベルの配列
	D3D_FEATURE_LEVEL levels[] = 
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	D3D_FEATURE_LEVEL featureLevel;
	for(size_t i = 0; i < _countof(levels); i++)
	{
		//採用したアダプターでデバイスを生成
		result = D3D12CreateDevice(tmpAdapter.Get(), levels[i], IID_PPV_ARGS(&device));
		assert(SUCCEEDED(result));
		if(result == S_OK)
		{
			//デバイスを生成出来た時点でループを抜ける
			featureLevel = levels[i];
			break;
		}
	}

	///コマンドリスト(GPUに、まとめて命令を送るためのコマンド)
	//コマンドアロケータを生成
	result = device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&commandAllocator)
	);
	assert(SUCCEEDED(result));

	//コマンドリストを生成
	result = device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(&commandList)
	);
	assert(SUCCEEDED(result));


	///コマンドキュー(コマンドリストをGPUに順位実行させていく仕組み)
	//コマンドキューの設定
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	//コマンドキューを生成
	result = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	assert(SUCCEEDED(result));


	///スワップチェーン(フロントバッファ、バックバッファを入れ替えてパラパラ漫画を作る)
	//設定
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = WinApp::window_width;
	swapChainDesc.Height = WinApp::window_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;				//色情報書式
	swapChainDesc.SampleDesc.Count = 1;								//マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;				//バックバッファ用
	swapChainDesc.BufferCount = 2;									//バッファ数を二つに設定
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;		//フリップ後は破棄
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	//生成
	//IDXGISwapChain1のComPtr用意
	ComPtr<IDXGISwapChain1> swapchain1;
	result = dxgiFactory->CreateSwapChainForHwnd(
		commandQueue.Get(),
		winApp->GetHwnd(),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapchain1
	);
	assert(SUCCEEDED(result));
	//生成したIDXGISwapChain1のオブジェクトをIDXGISwapChain4に変換する
	swapchain1.As(&swapChain);

	///レンダーターゲットビュー(バックバッファを描画キャンバスとして扱うオブジェクト)
	//デスクリプタヒープ生成(レンダーターゲットビューはデスクリプタヒープに生成するので準備)
	//設定
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;		//レンダーターゲットビュー
	rtvHeapDesc.NumDescriptors = swapChainDesc.BufferCount;	//表裏の二つ(ダブルバッファリング)
	//生成
	device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));

	//バックバッファ(スワップチェーン内で生成されたバックバッファのアドレス収容用)
	backBuffers.resize(swapChainDesc.BufferCount);

	//レンダーターゲットビュー(RTV)生成
	//スワップチェーンのすべてのバッファについて処理する
	for(size_t i = 0; i < backBuffers.size(); i++)
	{
		//スワップチェーンからバッファを取得
		swapChain->GetBuffer((UINT)i, IID_PPV_ARGS(&backBuffers[i]));
		//レンダーターゲットビューの設定
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		//シェーダーの計算結果をSRGBに変換して書き込む
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		//レンダーターゲットビューの生成
		device->CreateRenderTargetView
		(
			backBuffers[i].Get(),
			&rtvDesc,
			CD3DX12_CPU_DESCRIPTOR_HANDLE//デスクリプタヒープのハンドルを取得
			(//表か裏でアドレスがずれる
				rtvHeap->GetCPUDescriptorHandleForHeapStart(),
				i,
				device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type)
			)
		);
	}

	///深度バッファのリソース(テクスチャの一種)
	//リソース設定
	ComPtr<ID3D12Resource> depthBuff;
	CD3DX12_RESOURCE_DESC depthReourceDesc = CD3DX12_RESOURCE_DESC::Tex2D
		(
			DXGI_FORMAT_D32_FLOAT,
			WinApp::window_width,
			WinApp::window_height,
			1,0,
			1,0,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
		);
	//深度バッファの生成
	result = device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&depthReourceDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,	//深度値書き込みに使用
			&CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0),
			IID_PPV_ARGS(&depthBuff)
		);
	
	//深度ビュー用デスクリプタヒープ作成(DSV)
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.NumDescriptors = 1;			//深度ビューは一つ
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;	//デプスステンシルビュー
	result = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));
	assert(SUCCEEDED(result));
	//深度ビュー作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView(
		depthBuff.Get(),
		&dsvDesc,
		dsvHeap->GetCPUDescriptorHandleForHeapStart()
	);


	///フェンス(CPUとGPUで同期をとるための仕組み)
	//生成
	ComPtr<ID3D12Fence> fence;
	UINT64 fenceVal = 0;
	result = device->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(result));


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
	ComPtr<ID3D12Resource> vertBuff;
	result = device->CreateCommittedResource
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
	D3D12_VERTEX_BUFFER_VIEW vbView{};
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
	ComPtr<ID3D12Resource> indexBuff;
	result = device->CreateCommittedResource
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
	D3D12_INDEX_BUFFER_VIEW ibView{};
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeIB;



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
	ComPtr<ID3D12Resource> texBuff01 ;
	result= device->CreateCommittedResource
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
	result = device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap));
	assert(SUCCEEDED(result));

	///シェーダリソースビューの作成
	//設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};	//設定構造体
	srvDesc.Format = textureResourceDesc01.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;	//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = textureResourceDesc01.MipLevels;

	//ハンドルの指す位置にシェーダーリソースビューの作成
	device->CreateShaderResourceView
	(
		texBuff01.Get(),
		&srvDesc,
		CD3DX12_CPU_DESCRIPTOR_HANDLE//SRVヒープの先頭ハンドルを取得
			(
				srvHeap->GetCPUDescriptorHandleForHeapStart(),
				0,
				device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
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
	
	{		
		//配列内の全オブジェクトに対して
		for(size_t i = 0; i < _countof(object3ds); i++)
		{
			//初期化
			object3ds[0].color = {0.f, 0.f, 1.f, 1.f};
			InitializeObject3d(&object3ds[i], device.Get());

			//親子構造体
			//先頭以外
			if(i > 0)
			{
				//一つ前のオブジェクトを親とする
				//object3ds[i].parent = &object3ds[i - 1];

				//Scale
				object3ds[i].scale = {0.9f, 0.9f, 0.9f};
				//rotation
				object3ds[i].rotation = {0.0f, 0.0f, XMConvertToRadians(30.0f)};
				//position
				object3ds[i].position = {0.0f, 0.0f, 8.0f};
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
	}



	///ルートパラメータ
	//デスクリプタレンジの設定
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV;
	descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);


	//設定
	////定数バッファ 0番
	CD3DX12_ROOT_PARAMETER rootParam[3] = {};
	rootParam[0].InitAsConstantBufferView(0, 0);
	////テクスチャレジスタ 0番
	rootParam[1].InitAsDescriptorTable(1, &descRangeSRV);
	////定数バッファ 1番
	rootParam[2].InitAsConstantBufferView(1, 0);


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


	//ルートシグネチャ (テクスチャ、点数バッファなどシェーダーに渡すリソース情報をまとめたオブジェクト)
	//ルートシグネチャの生成
	ComPtr<ID3D12RootSignature> rootSignature;
	//設定
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_0(_countof(rootParam), rootParam,1, &samplerDesc,D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	//シリアライズ
	ComPtr<ID3DBlob> rootSigBlob;
	result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob,&errorBlob);
	assert(SUCCEEDED(result));
	result = device->CreateRootSignature(0,rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(),IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(result));
	//パイプラインにルートシグネチャをセット
	pipelineDesc.pRootSignature = rootSignature.Get();

	//パイプラインステート (グラフィックスパイプラインの設定をまとめたのがパイプラインステートオブジェクト(PSO))
	//パイプラインステートの生成
	ComPtr<ID3D12PipelineState> pipelineState ;
	result = device->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipelineState));
	assert(SUCCEEDED(result));



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



		///リソースバリア01
		//バックバッファの番号を取得(ダブルバッファなので 0 or 1)
		UINT bbIndex = swapChain->GetCurrentBackBufferIndex();
		
		//1. リソースバリアで書き込み可能に変更
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(backBuffers[bbIndex].Get(),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		///描画先指定コマンド
		//2. 描画先の変更
		commandList->OMSetRenderTargets
		(
			1,
			&CD3DX12_CPU_DESCRIPTOR_HANDLE//レンダーターゲットビューのハンドル取得
				(//表か裏でアドレスがずれる
					rtvHeap->GetCPUDescriptorHandleForHeapStart(),
					bbIndex,
					device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type)
				),
			false,
			&CD3DX12_CPU_DESCRIPTOR_HANDLE//深度ステンシルビュー用デスクリプタヒープのハンドル取得
				(
					dsvHeap->GetCPUDescriptorHandleForHeapStart(),
					0,
					device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
				)
		);


		///画面クリアコマンド
		//3. 画面クリア
		FLOAT clearColor[] = {0.1f, 0.25f, 0.5f, 0.0f};
		//色クリア
		commandList->ClearRenderTargetView
		(
			CD3DX12_CPU_DESCRIPTOR_HANDLE//レンダーターゲットビューのハンドル取得
				(//表か裏でアドレスがずれる
					rtvHeap->GetCPUDescriptorHandleForHeapStart(),
					bbIndex,
					device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type)
				),
			clearColor,
			0,
			nullptr
		);
		//深度クリア
		commandList->ClearDepthStencilView
		(
			CD3DX12_CPU_DESCRIPTOR_HANDLE//深度ステンシルビュー用デスクリプタヒープのハンドル取得
				(
					dsvHeap->GetCPUDescriptorHandleForHeapStart(),
					0,
					device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
				),
			D3D12_CLEAR_FLAG_DEPTH,
			1.0f,
			0,
			0,
			nullptr
		);


		///描画コマンド
		//4. 描画コマンド ここから

		///<summary>
		///	グラフィックスコマンド
		///<summary/>
		
		///ビューポート
		//設定コマンド
		commandList->RSSetViewports(1, &CD3DX12_VIEWPORT(0.0f, 0.0f, WinApp::window_width,WinApp::window_height));
		 
		///シザー矩形
		//設定
		commandList->RSSetScissorRects(1, &CD3DX12_RECT(0, 0, WinApp::window_width, WinApp::window_height));

		///パイプラインステートとルートシグネチャの設定コマンド
		commandList->SetPipelineState(pipelineState.Get());
		commandList->SetGraphicsRootSignature(rootSignature.Get());

		///プリミティブ形状
		//プリミティブ形状の設定コマンド
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//SRVヒープの設定コマンド	//１番目はSV
		ID3D12DescriptorHeap* ppHeaps[] = {srvHeap.Get()};
		commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
			
		commandList->SetGraphicsRootDescriptorTable
		(
			1, 
			CD3DX12_GPU_DESCRIPTOR_HANDLE//SRVヒープの先頭ハンドルを取得
			(
				srvHeap->GetGPUDescriptorHandleForHeapStart(),
				0,
				device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
			)
		);

		//全オブジェクトについて処理
		for(size_t i = 0; i < _countof(object3ds); i++)
		{
			DrawObject3d(&object3ds[i], commandList.Get(), vbView, ibView, _countof(indices));
		}


		//4. 描画コマンド ここまで


		///リソースバリア02
		//5. リソースバリアを戻す
		commandList->ResourceBarrier(1,&CD3DX12_RESOURCE_BARRIER::Transition(backBuffers[bbIndex].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));


		///コマンドのフラッシュ
		//命令クローズ
		result = commandList->Close();
		assert(SUCCEEDED(result));
		//コマンドリストの実行
		ID3D12CommandList* commandLists[] = {commandList.Get()};
		commandQueue->ExecuteCommandLists(1, commandLists);
		
		//画面に表示するバッファをフリップ(表裏の入れ替え)
		result = swapChain->Present(1, 0);
		assert(SUCCEEDED(result));


		///コマンド完了待ち
		//コマンドの実行完了を待つ
		commandQueue->Signal(fence.Get(), ++fenceVal);
		if(fence->GetCompletedValue() != fenceVal)
		{
			HANDLE event = CreateEvent(nullptr, false, false, nullptr);
			fence->SetEventOnCompletion(fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}

		//キュークリア
		result = commandAllocator->Reset();
		assert(SUCCEEDED(result));
		//再びコマンドリストを溜める準備
		result = commandList->Reset(commandAllocator.Get(), nullptr);
		assert(SUCCEEDED(result));


		/// <summary>
		/// DirectX12 毎フレーム処理 ここまで
		/// </summary>

	}
	delete input;

	//ゲームウィンドウ破棄
	delete winApp;

	return 0;
}


void InitializeObject3d(Object3d *object, ID3D12Device* device)
{
	HRESULT result;
	//定数バッファのヒープ設定
	D3D12_HEAP_PROPERTIES heapProp{};
	heapProp.Type =D3D12_HEAP_TYPE_UPLOAD;
	//定数バッファのリソース設定
	D3D12_RESOURCE_DESC resDesc{};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = (sizeof(ConstBufferDataMaterial) + 0xff) & ~0xff;	//256バイトアライメント
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//生成
	result = device->CreateCommittedResource(
		&heapProp,			//ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&resDesc,		//リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&object->constBufferMaterial)
	);
	assert(SUCCEEDED(result));

	///定数バッファのマッピング(GPUのVRAMが、CPUのメインメモリに連動)
	result = object->constBufferMaterial->Map(0, nullptr, (void**)&object->constMapMaterial);
	assert(SUCCEEDED(result));
	
	///定数バッファへのデータ転送
	//値を書き込むと自動的に転送される
	object->constMapMaterial->color = object->color;



	resDesc.Width = (sizeof(ConstBufferDataTransform)+ 0xff) & ~0xff;
	//生成
	result = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&object->constBuffTransform));
	assert(SUCCEEDED(result));

	//定数バッファのマッピング
	result = object->constBuffTransform->Map(0,nullptr, (void**)&object->constMapTransform);
	assert(SUCCEEDED(result));
}

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
	object->constMapTransform->mat = object->matWorld * matView *matProjection;
}

void DrawObject3d(Object3d *object, ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW &vbView, D3D12_INDEX_BUFFER_VIEW &ibView, UINT numIndices)
{

	///定数バッファビュー
	//定数バッファビュー(CBV)の設定コマンド	//0番目はCBV
	commandList->SetGraphicsRootConstantBufferView(0, object->constBufferMaterial->GetGPUVirtualAddress());

	//頂点バッファの設定
	commandList->IASetVertexBuffers(0, 1, &vbView);
	//インデックスバッファの設定
	commandList->IASetIndexBuffer(&ibView);
	//定数バッファビュー(CBVの設定コマンド)
	commandList->SetGraphicsRootConstantBufferView(2, object->constBuffTransform->GetGPUVirtualAddress());
	//描画コマンド
	commandList->DrawIndexedInstanced(numIndices,1, 0, 0, 0);
}
