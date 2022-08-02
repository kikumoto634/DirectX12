#include "ViewProjection.h"
#include <d3dx12.h>
#include "DirectXCommon.h"

using namespace DirectX;

void ViewProjection::Initialize()
{
	CreateConstBuffer();
	Map();
	UpdateMatrix();
}

void ViewProjection::CreateConstBuffer()
{
	HRESULT result = S_FALSE;

	//ヒーププロパティ
	CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	//リソース設定
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferViewProjection) + 0xff) & ~0xff);

	//定数バッファ生成
	result = DirectXCommon::GetInstance()->GetDevice()->CreateCommittedResource
	(
		&heapProp, D3D12_HEAP_FLAG_NONE, 
		&resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&constBuff)
	);
	assert(SUCCEEDED(result));
}

void ViewProjection::Map()
{
	//定数バッファとのデータリンク
	HRESULT result = S_FALSE;
	result = constBuff->Map(0,nullptr, (void**)&constBuff);
	assert(SUCCEEDED(result));
}

void ViewProjection::UpdateMatrix()
{
	//ビュー行列の生成
	matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));

	//透視投影による射影行列の生成
	matProjection = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, nearZ, farZ);

	//定数バッファ
	constMap->view = matView;
	constMap->projection = matProjection;
	constMap->cameraPos = eye;
}