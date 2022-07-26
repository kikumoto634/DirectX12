#pragma once

#include <DirectXMath.h>

/// <summary>
/// カメラ
/// </summary>
class Camera
{
public:
	using XMMATRIX = DirectX::XMMATRIX;
	using XMFLOAT3 = DirectX::XMFLOAT3;

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

	const XMFLOAT3& GetEye() {return eye; }
	const XMFLOAT3& GetTarget() {return target; }
	const XMFLOAT3& GetUp() {return up; }

	//setter
	void SetEye(const XMFLOAT3& eye)	{this->eye = eye; }
	void SetTarget(const XMFLOAT3& target)	{this->target = target; }
	void SetUp(const XMFLOAT3& up)	{this->up = up; }

	void CameraMovement(XMFLOAT3 pos);

protected:

	//透視投影
	XMMATRIX matProjection;	//プロジェクション行列
	//ビュー変換行列
	XMMATRIX matView;		//ビュー行列
	XMFLOAT3 eye;			//視点座標
	XMFLOAT3 target;		//注視点座標
	XMFLOAT3 up;			//上方向ベクトル
};

