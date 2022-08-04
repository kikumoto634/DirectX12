#pragma once
#include <DirectXMath.h>
#include <string>
#include <d3d12.h>
#include <wrl.h>

/// <summary>
/// スプライト
/// </summary>
class Sprite
{
public:	//エイリアス

	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;

public:	//サブクラス
	/// <summary>
	/// 頂点座標
	/// </summary>
	struct VertexPosUv
	{
		XMFLOAT3 pos;
		XMFLOAT2 uv;
	};

	/// <summary>
	/// 定数バッファ
	/// </summary>
	struct ConstBufferData
	{
		XMFLOAT4 color;
		XMMATRIX mat;
	};

public:	//静的メンバ関数

	/// <summary>
	/// 静的初期化
	/// </summary>
	/// <param name="device">デバイス</param>
	/// <param name="window_width">画面幅</param>
	/// <param name="window_height">画面高さ</param>
	static void StaticInitialize(ID3D12Device* device,int window_width, int window_height, const std::wstring& directoryPath = L"Resources/");

	/// <summary>
	/// 描画前処理
	/// </summary>
	/// <param name="cmdList">描画コマンドリスト</param>
	static void PreDraw(ID3D12GraphicsCommandList* commandList);

	/// <summary>
	/// 描画後処理
	/// </summary>
	static void PostDraw();

	/// <summary>
	/// スプライト生成
	/// </summary>
	/// <param name="texNumber">テクスチャハンドル</param>
	/// <param name="position">座標</param>
	/// <param name="color">色</param>
	/// <param name="anchorpoint">アンカーポイント</param>
	/// <param name="isFlipX">左右反転</param>
	/// <param name="isFlipY">上下反転</param>
	/// <returns>生成されたスプライト</returns>
	static Sprite* Create(uint32_t textureHandle, XMFLOAT2 pos, XMFLOAT4 color = {1,1,1,1}, XMFLOAT2 anchor = {0.f,0.f}, bool isFlipX = false, bool isFlipY = false);

private: //静的メンバ関数
	//頂点数
	static const int verticesNum = 4;
	//デバイス
	static ID3D12Device* sDevice;
	//デストラクタサイズ
	static UINT sDescriptorHandleSize;
	//グラフィックスコマンドリスト
	static ID3D12GraphicsCommandList* sCommandList;
	//ルートシグネチャ
	static ComPtr<ID3D12RootSignature> sRootSignature;
	//パイプライン
	static ComPtr<ID3D12PipelineState> sPipelineState;
	//射影行列
	static XMMATRIX sMatProjection;

public:	//メンバ関数

	Sprite() = default;

	/// <summary>
	/// コンストラクタ
	/// </summary>
	Sprite(uint32_t textureHandle, XMFLOAT2 pos, XMFLOAT2 size, XMFLOAT4 color, XMFLOAT2 anchor, bool isFlipX, bool isFlipY);

	///<summary>
	///初期化
	/// </summary>
	bool Initialize();

	/// <summary>
	///	テクスチャハンドル
	/// </summary>
	/// <param name="textureHandle">テクスチャ番号</param>
	void SetTextureHandle(uint32_t textureHandle);

	uint32_t GetTextureHandle()	{return textureHandle;}

	/// <summary>
	/// 座標設定
	/// </summary>
	/// <param name="pos">座標</param>
	void SetPosition(const XMFLOAT2& pos);

	const XMFLOAT2& GetPosition()	{return position;}

	/// <summary>
	/// 角度の設定
	/// </summary>
	/// <param name="rotation">角度</param>
	void SetRotation(const float rotation);

	float GetRotation() { return rotation; }

	/// <summary>
	/// サイズの設定
	/// </summary>
	/// <param name="size">サイズ</param>
	void SetSize(const XMFLOAT2& size);

	const XMFLOAT2& GetSize() { return size; }

	/// <summary>
	/// アンカーポイントの設定
	/// </summary>
	/// <param name="anchorpoint">アンカーポイント</param>
	void SetAnchorPoint(const XMFLOAT2& anchorpoint);

	const XMFLOAT2& GetAnchorPoint() { return anchorPoint; }

	/// <summary>
	/// 色の設定
	/// </summary>
	/// <param name="color">色</param>
	void SetColor(const XMFLOAT4& color) { this->color = color; };

	const XMFLOAT4& GetColor() { return color; }

	/// <summary>
	/// 左右反転の設定
	/// </summary>
	/// <param name="isFlipX">左右反転</param>
	void SetIsFlipX(const bool isFlipX);

	bool GetIsFlipX() { return isFlipX; }

	/// <summary>
	/// 上下反転の設定
	/// </summary>
	/// <param name="isFlipX">上下反転</param>
	void SetIsFlipY(const bool isFlipY);

	bool GetIsFlipY() { return isFlipY; }

	/// <summary>
	/// テクスチャ範囲設定
	/// </summary>
	/// <param name="texBase">テクスチャ左上座標</param>
	/// <param name="texSize">テクスチャサイズ</param>
	void SetTextureRect(const XMFLOAT2& texBase, const XMFLOAT2& texSize);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

private: // メンバ関数
	/// <summary>
	/// 頂点データ転送
	/// </summary>
	void TransferVertices();

  private: // メンバ変数
	// 頂点バッファ
	ComPtr<ID3D12Resource> vertBuff;
	// 定数バッファ
	ComPtr<ID3D12Resource> constBuff;
	// 頂点バッファマップ
	VertexPosUv* vertMap = nullptr;
	// 定数バッファマップ
	ConstBufferData* constMap = nullptr;
	// 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	// テクスチャ番号
	UINT textureHandle = 0;
	// Z軸回りの回転角
	float rotation = 0.0f;
	// 座標
	XMFLOAT2 position{};
	// スプライト幅、高さ
	XMFLOAT2 size = {100.0f, 100.0f};
	// アンカーポイント
	XMFLOAT2 anchorPoint = {0, 0};
	// ワールド行列
	XMMATRIX matWorld{};
	// 色
	XMFLOAT4 color = {1, 1, 1, 1};
	// 左右反転
	bool isFlipX = false;
	// 上下反転
	bool isFlipY = false;
	// テクスチャ始点
	XMFLOAT2 texBase = {0, 0};
	// テクスチャ幅、高さ
	XMFLOAT2 texSize = {100.0f, 100.0f};
	// リソース設定
	D3D12_RESOURCE_DESC resourceDesc;
};