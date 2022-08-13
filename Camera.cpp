#include "Camera.h"
#include "WinApp.h"

using namespace DirectX;

void Camera::Initialize()
{
	const float distance = 100.f;	//カメラの距離

	eye = {0, 0, -distance};
	target = {0, 0, 0};
	up = {0, 1, 0};

	nearZ = 0.1f;
	farZ = 1000.f;

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
	//matView = MatrixLookAtLH(eye, target, up);

	XMFLOAT3 keye{eye.x, eye.y, eye.z};
	XMFLOAT3 ktarget{target.x, target.y, target.z};
	XMFLOAT3 kup{up.x, up.y, up.z};
	matView = XMMatrixLookAtLH(XMLoadFloat3(&keye), XMLoadFloat3(&ktarget), XMLoadFloat3(&kup));
}

void Camera::UpdateProjectionMatrix()
{
	//透視投影
	/*matProjection = MatrixProjectionMatrix(
		XMConvertToRadians(45.f),
		(float)WinApp::window_width / WinApp::window_height,
		nearZ, farZ
	);*/
	matProjection = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(45.0f),	//上下画角45°
		(float)WinApp::window_width / WinApp::window_height,			//aspect比(画面横幅/画面縦幅)
		nearZ, farZ				//前端、奥端
	);
}

void Camera::CameraMovement(Vector3 pos)
{
	eye += pos;
	target += pos;
}

