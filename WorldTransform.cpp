#include "WorldTransform.h"
#include "DirectXCommon.h"
#include <cassert>
#include <d3d12.h>

using namespace DirectX;

void WorldTransform::Initialize()
{
	CreateConstBuffer();
	Map();
	UpdateMatrix();
}

void WorldTransform::CreateConstBuffer()
{
	HRESULT result = S_FALSE;

	//ヒーププロパティ
	CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	//リソース設定
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferDataWorldTransform) + 0xff) & ~0xff);

	//定数バッファの生成
	result = DirectXCommon::GetInstance()->GetDevice()->CreateCommittedResource
		(
			&heapProp,  D3D12_HEAP_FLAG_NONE,
			&resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(&constBuff)
		);
	assert(SUCCEEDED(result));
}

void WorldTransform::Map()
{
	//定数バッファマッピング
	HRESULT result = S_FALSE;
	result = constBuff->Map(0, nullptr, (void**)&constMap);
	assert(SUCCEEDED(result));
}

void WorldTransform::UpdateMatrix()
{
	XMMATRIX matScale, matRot, matTrans;

	//行列計算
	matScale = XMMatrixScaling(scale.x, scale.y, scale.z);
	matRot *= XMMatrixRotationZ(rotation.z);
	matRot *= XMMatrixRotationX(rotation.x);
	matRot *= XMMatrixRotationY(rotation.y);
	matTrans = XMMatrixTranslation(position.x, position.y, position.z);

	//ワールド行列
	matWorld =XMMatrixIdentity();
	matWorld *= matScale;
	matWorld *= matRot;
	matWorld *= matTrans;

	//親
	if(parent)
	{
		matWorld *= parent->matWorld;
	}

	//定数バッファ
	constMap->matWorld = matWorld;
}
