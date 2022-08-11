#pragma once

#include <DirectXMath.h>
#include "Vector3.h"

/// <summary>
/// カメラ
/// </summary>
class Camera
{
public:
	using XMMATRIX = DirectX::XMMATRIX;

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	virtual void Update();

	void UpdateViewMatrix();

	void UpdateProjectionMatrix();


	//getter
	const XMMATRIX& GetMatProjection()	{return matProjection;}
	const XMMATRIX& GetMatView()	{return matView;}
	const XMMATRIX& GetViewProjectionMatrix()	{return matViewProjection;}

	const Vector3& GetUp() {return up; }

	/// <summary>
	/// カメラ位置移動
	/// </summary>
	/// <param name="pos"></param>
	void CameraMovement(Vector3 pos);

	/// <summary>
	/// 注視点座標移動
	/// </summary>
	const Vector3& GetTarget() {return target; }
	void SetTarget(Vector3 target)	{this->target = target;};
	
	/// <summary>
	/// 視点座標
	/// </summary>
	const Vector3& GetEye() {return eye; }
	void SetEye(Vector3 eye)	{this->eye = eye;};

	/// <summary>
	/// ニアクリップ移動
	/// </summary>
	const float& GetNearZ() {return nearZ; }
	void SetNearZ(float nearZ)	{this->nearZ = nearZ;};

	/// <summary>
	/// ファークリップ移動
	/// </summary>
	const float& GetFarZ() {return farZ; }
	void SetFarZ(float farZ)	{this->farZ = farZ;};

protected:

	//透視投影
	XMMATRIX matProjection;	//プロジェクション行列
	//ビュー変換行列
	XMMATRIX matView;		//ビュー行列
	Vector3 eye;			//視点座標
	Vector3 target;		//注視点座標
	Vector3 up;			//上方向ベクトル

	float nearZ;
	float farZ;

	XMMATRIX matViewProjection;
};

