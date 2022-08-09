#pragma once
#include <DirectXMath.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <wrl.h>
#include <string>
#include <vector>
#include <fbxsdk.h>
#include <DirectXTex.h>

struct NODE
{
	//名前
	std::string name;
	//ローカルスケール
	DirectX::XMVECTOR scaling = {1, 1, 1, 0};
	//ローカル回転角
	DirectX::XMVECTOR rotation = {0,0, 0, 0};
	//ローカル移動
	DirectX::XMVECTOR translation = {0, 0, 0, 1};
	//ローカル変形行列
	DirectX::XMMATRIX transform;
	//グローバル変形行列
	DirectX::XMMATRIX globalTransform;
	//親ノード
	NODE* parent = nullptr;
};


//ボーン構造体
struct BONE
{
	//名前
	std::string name;
	//初期姿勢の逆行列
	DirectX::XMMATRIX invInitialPose;
	//クラスター(FBX側のボーン情報)
	FbxCluster* fbxCluster;
	//コンストラクタ
	BONE(const std::string& name)
	{
		this->name = name;
	}
};

/// <summary>
/// Fbxモデル
/// </summary>
class FbxModelMesh
{
public: //定数
	static const int MAX_BONE_INDICES = 4;

private:	//エイリアス
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;

public: //サブクラス
	friend class FbxLoder;

	//頂点情報
	struct VertexPosNormalUVSkin
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT3 uv;
		UINT boneIndex[MAX_BONE_INDICES];
		float boneWeight[MAX_BONE_INDICES];
	};

public: //メンバ関数

	void CreateBuffers();

	void Draw(ID3D12GraphicsCommandList* commandList, UINT rootParameterIndex);

private: //メンバ変数

	FbxScene* fbxScene = nullptr;

	std::string name;

	std::vector<NODE> nodes;

	//メッシュを持つノード
	NODE* meshNode= nullptr;


	//頂点バッファ
	ComPtr<ID3D12Resource> vertBuff;
	//インデックスバッファ
	ComPtr<ID3D12Resource> indexBuff;
	//テクスチャバッファ
	ComPtr<ID3D12Resource> texBuff;
	//頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	//インデックスバッファビュー
	D3D12_INDEX_BUFFER_VIEW ibView{};
	//SRV用デスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> descHeapsSRV;

	//頂点データ配列
	std::vector<VertexPosNormalUVSkin> vertices;
	//インデックス
	std::vector<unsigned short> indices;

	///マテリアル
	XMFLOAT3 ambient = {1,1,1};
	XMFLOAT3 diffuse = {1,1,1};
	DirectX::TexMetadata metaData = {};
	DirectX::ScratchImage scratchImg = {};

	//ボーン
	std::vector<BONE> bones;
};

