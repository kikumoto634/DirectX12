#include "WorldTransform.h"
#include "DirectXCommon.h"

#include <cassert>
#include <d3dx12.h>

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

	//リソース設定

	//定数バッファ生成
}

void WorldTransform::Map()
{

}

void WorldTransform::UpdateMatrix()
{

}