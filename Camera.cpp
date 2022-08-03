#include "Camera.h"
#include "WinApp.h"

using namespace DirectX;

void Camera::Initialize()
{
	const float distance = 100.f;	//カメラの距離

	eye = {0, 0, -distance};
	target = {0, 0, 0};
	up = {0, 1, 0};

	UpdateViewMatrix();
	UpdateProjectionMatrix();
}

void Camera::Update()
{
	matViewProjection = matView * matProjection;

	UpdateViewMatrix();
	UpdateProjectionMatrix();
}

void Camera::UpdateViewMatrix()
{
	//ビュー変換行列
	matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));	
}

void Camera::UpdateProjectionMatrix()
{
	//透視投影
	//matProjection = XMMatrixPerspectiveFovLH(
	//	XMConvertToRadians(45.0f),	//上下画角45°
	//	(float)WinApp::window_width / WinApp::window_height,			//aspect比(画面横幅/画面縦幅)
	//	0.1f, 1000.0f				//前端、奥端
	//);
}

void Camera::CameraMovement(XMFLOAT3 pos)
{
	eye.x += pos.x;
	eye.y += pos.y;
	eye.z += pos.z;

	target.x += pos.x;
	target.y += pos.y;
	target.z += pos.z;
}
