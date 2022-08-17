#pragma once
#include <DirectXMath.h>

#include "Vector3.h"
#include "Matrix4.h"


/// <summary>
/// レールカメラ
/// </summary>
class RailCamera
{
public:
	void Initialize(Vector3 pos, Vector3 rot);
	void Update(Vector3 pos, Vector3 rot);

private:
	void UpdateViewMatrix();
	void UpdateProjectionMatrix();

	Matrix4 MatricChange(DirectX::XMMATRIX mat);

private:
	//透視投影
	DirectX::XMMATRIX matProjection;	//プロジェクション行列
	//ビュー変換行列
	DirectX::XMMATRIX matView;		//ビュー行列
	Vector3 eye;			//視点座標
	Vector3 target;		//注視点座標
	Vector3 up;			//上方向ベクトル

	float nearZ;
	float farZ;

	DirectX::XMMATRIX matViewProjection;
};

