#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <string>

/// <summary>
/// スプライト
/// </summary>
class Sprite
{
public://サブクラス

	//スプライトデータ構造
	struct VertexPosUv
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 uv;
	};

	//定数バッファ用データ構造体(3D変換行列
	struct ConstBufferData{
		DirectX::XMMATRIX mat;	//3D変換行列
		DirectX::XMFLOAT4 color;	//色(RGBA)
	};

private: // 静的メンバ変数
	// 頂点数
	static const int kVertNum = 4;
	// デバイス
	static ID3D12Device* sDevice_;
	// デスクリプタサイズ
	static UINT sDescriptorHandleIncrementSize_;
	// コマンドリスト
	static ID3D12GraphicsCommandList* sCommandList_;
	// ルートシグネチャ
	static Microsoft::WRL::ComPtr<ID3D12RootSignature> sRootSignature_;
	//パイプラインステートオブジェクト
	static Microsoft::WRL::ComPtr<ID3D12PipelineState> sPipelineStates_;
	// 射影行列
	static DirectX::XMMATRIX sMatProjection_;

	static void InitializeGraphicsPipeline();

public://静的メンバ関数
	/// <summary>
	/// 静的メンバの初期化
	/// </summary>
	static void StaticInitialize(ID3D12Device* device, int window_width, int window_height, const std::wstring& directoryPath = L"Resources/");

	/// <summary>
	/// 描画前処理
	/// </summary>
	/// <param name="commandList">コマンドリスト</param>
	static void PreDraw(ID3D12GraphicsCommandList* commandList);

	/// <summary>
	/// 描画後処理
	/// </summary>
	static void PostDraw();

	/// <summary>
	/// スプライト生成
	/// </summary>
	/// <param name="textureHandle">テクスチャハンドル</param>
	/// <param name="position">座標</param>
	/// <param name="color">色</param>
	/// <param name="anchorpoint">アンカーポイント</param>
	/// <param name="isFlipX">左右反転</param>
	/// <param name="isFlipY">上下反転</param>
	/// <returns>生成されたスプライト</returns>
	static Sprite* Create(uint32_t textureHandle, DirectX::XMFLOAT2 position, DirectX::XMFLOAT4 color = {1,1,1,1}, DirectX::XMFLOAT2 anchorpoint = {0.f,0.f}, bool isFlipX = false, bool isFlipY = false);


public://メンバ関数

	/// <summary>
	/// コンストラクタ
	/// </summary>
	Sprite() = default;

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="textureHandle">テクスチャハンドル</param>
	/// <param name="position">座標</param>
	/// <param name="size">サイズ</param>
	/// <param name="color">色</param>
	/// <param name="anchorpoint">アンカーポイント</param>
	/// <param name="isFlipX">左右反転</param>
	/// <param name="isFlipY">上下反転</param>
	Sprite(uint32_t textureHandle, DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 color, DirectX::XMFLOAT2 anchorPoint, bool isFlipX, bool isFlipY);

	/// <summary>
	/// 初期化
	/// </summary>
	bool Initialize();

	/// <summary>
	/// 頂点データ転送
	/// </summary>
	void TransferVertices();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// テクスチャハンドル設定
	/// </summary>
	/// <param name="textureHandle">テクスチャハンドル</param>
	void SetTextureHandle(const uint32_t textureHandle);

	const uint32_t GetTextureHandle()	{return textureHandle;}

	/// <summary>
	/// 座標設定
	/// </summary>
	/// <param name="position"></param>
	void SetPosition(const DirectX::XMFLOAT2& position);

	const DirectX::XMFLOAT2& GetPosition()	{return position;}

	/// <summary>
	/// 回転角設定
	/// </summary>
	/// <param name="rotation"></param>
	void SetRotation(const float rotation);

	const float GetRotation()	{return rotation;}

	/// <summary>
	/// サイズ設定
	/// </summary>
	/// <param name="size">サイズ</param>
	void SetSize(const DirectX::XMFLOAT2& size);

	const DirectX::XMFLOAT2 GetSize()	{return size;}

	/// <summary>
	/// アンカーポイント設定
	/// </summary>
	/// <param name="anchorpoint"></param>
	void SetAnchorPoint(const DirectX::XMFLOAT2& anchorpoint);

	const DirectX::XMFLOAT2& GetAnchorPoint()	{return anchorPoint;}

	/// <summary>
	/// 色の設定
	/// </summary>
	/// <param name="color">色</param>
	void SetColor(const DirectX::XMFLOAT4& color) { this->color = color; };

	const DirectX::XMFLOAT4& GetColor() { return color; }

	/// <summary>
	/// 左右反転の設定
	/// </summary>
	/// <param name="isFlipX">左右反転</param>
	void SetIsFlipX(const bool isFlipX);

	const bool GetIsFlipX() { return isFlipX; }

	/// <summary>
	/// 上下反転の設定
	/// </summary>
	/// <param name="isFlipX">上下反転</param>
	void SetIsFlipY(bool isFlipY);

	const bool GetIsFlipY() { return isFlipY; }

	/// <summary>
	/// テクスチャ範囲設定
	/// </summary>
	/// <param name="texBase">テクスチャ左上座標</param>
	/// <param name="texSize">テクスチャサイズ</param>
	void SetTextureRect(const DirectX::XMFLOAT2& texBase, const DirectX::XMFLOAT2& texSize);


private://メンバ変数
	// 頂点バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> vertBuff;
	// 定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> constBuff;
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
	DirectX::XMFLOAT2 position{};
	// スプライト幅、高さ
	DirectX::XMFLOAT2 size = {100.0f, 100.0f};
	// アンカーポイント
	DirectX::XMFLOAT2 anchorPoint = {0, 0};
	// ワールド行列
	DirectX::XMMATRIX matWorld{};
	// 色
	DirectX::XMFLOAT4 color = {1, 1, 1, 1};
	// 左右反転
	bool isFlipX = false;
	// 上下反転
	bool isFlipY = false;
	// テクスチャ始点
	DirectX::XMFLOAT2 texBase = {0, 0};
	// テクスチャ幅、高さ
	DirectX::XMFLOAT2 texSize = {100.0f, 100.0f};
	// リソース設定
	D3D12_RESOURCE_DESC resourceDesc;
};

